#include "board.h"
#include "common.h"
#include "_crc.h"
#include "_misc.h"
#include "fd_dsp.h"
#include "modParser.h"
#include "modPaint.h"
#include "halPaint.h"
#include "modGUI.h"
#include "xprintf.h"

#include "mavlink.h"
#include "conf.h"
#include "mavlink_msg_fd.h"
#include "mavlink_msg_fd_clb.h"

#include "lvgl.h"
#include "TFT_eSPI.h" // Ensure TFT_eSPI is compatible with ESP-IDF or use an alternative
#include "touch.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "sdmmc_cmd.h"

#include "Audio.h"
Audio audio;

// Define pins according to your ESP32 board
#define SD_MOSI GPIO_NUM_11
#define SD_MISO GPIO_NUM_13
#define SD_SCK  GPIO_NUM_12
#define SD_CS   GPIO_NUM_10

#define I2S_DOUT GPIO_NUM_17
#define I2S_BCLK GPIO_NUM_42
#define I2S_LRC  GPIO_NUM_18

// Initialize TFT display
TFT_eSPI tft = TFT_eSPI(); // Ensure compatibility with ESP-IDF

// LVGL Display Buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[LV_HOR_RES_MAX * 10];
static lv_color_t buf2[LV_HOR_RES_MAX * 10];
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

// LVGL Canvas Object
lv_obj_t *canvas;

// Function Prototypes
void my_flush_cb(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p);
bool my_touch_read(lv_indev_drv_t * drv, lv_indev_data_t * data);
void setup_lvgl();

// Global Variables
CHAR str[256];

#include "_filter.h"
filter_window_FLOAT_t filt_0;

parser_t parser_rx;
U8 parser_rx_buf[ PARSER_MAX_DATA_SIZE ];
parser_t parser_tx;
U8 parser_tx_buf[ PARSER_MAX_DATA_SIZE ];
U32 ver = 20240407;
U32 serial_chars_cnt = 0;
U32 mavlink_msg_cnt = 0;
U32 mavps = 0;
fd_data_t fd_data;
U32 mode = 3;
U32 filter_mode = 1;
U32 mul = 1;
fd_serial_data_t fsd;
FLOAT angle = 0.;

static FLOAT calibrate_buf[ FD_CLB_DOTS * 2 ];
INT mavlink_send_state = 0;

// Function Implementations

void my_flush_cb(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p) {
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, area->x2 - area->x1 +1, area->y2 - area->y1 +1);
    tft.pushColors((uint16_t *)&color_p->full, (area->x2 - area->x1 +1) * (area->y2 - area->y1 +1), true);
    tft.endWrite();
    lv_disp_flush_ready(disp);
}

bool my_touch_read(lv_indev_drv_t * drv, lv_indev_data_t * data) {
    if(touch_touched()) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touch_last_x;
        data->point.y = touch_last_y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
    return false; // No more data to read
}

void setup_lvgl() {
    lv_init();

    tft.begin();
    tft.setRotation(1); // Set desired orientation

    // Initialize LVGL buffer
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LV_HOR_RES_MAX * 10);

    // Initialize LVGL display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 800;
    disp_drv.ver_res = 480;
    disp_drv.flush_cb = my_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.rounder_cb = NULL;
    disp_drv.vsync_cb = NULL;
    lv_disp_drv_register(&disp_drv);

    // Initialize LVGL input device driver for touch
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touch_read;
    lv_indev_drv_register(&indev_drv);

    // Create Canvas for drawing
    canvas = lv_canvas_create(lv_scr_act(), NULL);
    lv_canvas_set_buffer(canvas, buf1, LV_HOR_RES_MAX, LV_VER_RES_MAX, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_pos(canvas, 0, 0);
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
}

// Sound Play Function
void sound_play(const char *str) {
    audio.connecttoFS(SD, str);
    do {
        // Implement non-blocking loop if possible
        audio.loop();
        vTaskDelay(pdMS_TO_TICKS(10)); // Yield to other tasks
    } while(audio.isRunning());
}

// Touch Functions
int fd_dsp_touch_has_signal(void) {
    return touch_has_signal();
}

int fd_dsp_touch_touched(void) {
    return touch_touched();
}

int fd_dsp_touch_released(void) {
    return touch_released();
}

// Button Test Function
INT fd_dsp_button_test(button_t *b) {
    INT ret = 0;

    if(touch_last_x >= b->x && touch_last_x <= (b->x + b->w)) {
        if(touch_last_y >= b->y && touch_last_y <= (b->y + b->h)) {
            ret = 1;
        }
    }

    if(ret == 1) { // Send sound
        sound_play("/0001_blip1.mp3");
    }

    return ret;
}

// Calibration Functions
void fd_dsp_calibrate_reset(void) {
    if(mavlink_send_state == CMD_NOP)
        mavlink_send_state = CMD_FLASH_ERASE;
}

void fd_dsp_calibrate_write(void) {
    if(mavlink_send_state == CMD_NOP)
        mavlink_send_state = CMD_FLASH_WRITE;
}

void fd_dsp_calibrate_send_process(void) {
    mavlink_message_t msg;
    mavlink_fd_clb_t mav_fd_clb;
    static INT iter = 0;
    static INT delay_cnt = 0;
    CHAR str_local[32];

    switch(mavlink_send_state) {
        case CMD_NOP: {
            iter = 0;
        } break;

        case CMD_FLASH_ERASE: {
            xsprintf(str_local, "Sending erase...");
            lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
            lv_label_set_text(label, str_local);
            lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
            lv_obj_set_pos(label, 310, 20);

            if(delay_cnt == 0) {
                mav_fd_clb.cmd = CMD_FLASH_ERASE;
                mav_fd_clb.data_num = 0;
                mav_fd_clb.idnum = fsd.idnum;

                mavlink_msg_fd_clb_encode(SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb);
                INT size = mavlink_msg_to_send_buffer(parser_tx_buf, &msg);

                // Send via UART or appropriate interface
                uart_write_bytes(UART_NUM_1, (const char*)parser_tx_buf, size);
            }
            delay_cnt++;
            if(delay_cnt == 30) {
                delay_cnt = 0;
                mavlink_send_state = CMD_NOP;
            }
        } break;

        case CMD_FLASH_WRITE: {
            xsprintf(str_local, "Sending %u...", iter);
            lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
            lv_label_set_text(label, str_local);
            lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
            lv_obj_set_pos(label, 310, 20);

            delay_cnt++;
            if(delay_cnt == 5) {
                delay_cnt = 0;
                if(iter < 360 / 8) {
                    mav_fd_clb.cmd = CMD_FLASH_WRITE;
                    mav_fd_clb.data_num = iter;
                    mav_fd_clb.idnum = fsd.idnum;

                    for(INT j = 0; j < 8; j++) {
                        mav_fd_clb.clb_val[j] = calibrate_buf[iter*8 + j];
                    }

                    mavlink_msg_fd_clb_encode(SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb);
                    INT size = mavlink_msg_to_send_buffer(parser_tx_buf, &msg);

                    uart_write_bytes(UART_NUM_1, (const char*)parser_tx_buf, size);
                }
                else {
                    mavlink_send_state = CMD_FLASH_FINALIZE;
                }
                iter++;
            }
        } break;

        case CMD_FLASH_FINALIZE: {
            xsprintf(str_local, "Sending finished...");
            lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
            lv_label_set_text(label, str_local);
            lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
            lv_obj_set_pos(label, 310, 20);

            mav_fd_clb.cmd = CMD_FLASH_FINALIZE;
            mav_fd_clb.data_num = 0;
            mav_fd_clb.idnum = fsd.idnum;

            mavlink_msg_fd_clb_encode(SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb);
            INT size = mavlink_msg_to_send_buffer(parser_tx_buf, &msg);

            uart_write_bytes(UART_NUM_1, (const char*)parser_tx_buf, size);

            mavlink_send_state = CMD_NOP;
        } break;

        default: break;
    }
}

// Drawing Function
void drawfunc(void) {
    static U32 sec = 0;
    static U32 psec = 0;
    sec = millis() / (1000 / 20); // Update 20 times per second
    if(psec != sec) {
        psec = sec;

        // Clear Canvas
        lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

        // Draw DSP data
        fd_dsp_draw_all(&fd_data, &fsd, mode, filter_mode, mul);
        fd_dsp_buttons_processor(&mode, &filter_mode, &mul);

        // Display FPS and MAVPS
        xsprintf(str, "fps:%d m:%u", fps, mavps);
        lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
        lv_label_set_text(label, str);
        lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_pos(label, 620, 2);

        frame_count++;
    }
}

// Setup Function
void app_main(void) {
    // Initialize NVS (required for some components)
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize UART
    const uart_config_t uart_config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, 4096, 0, 0, NULL, 0);

    // Initialize Touch
    touch_init();

    // Initialize LVGL
    setup_lvgl();

    // Initialize SD Card
    gpio_set_direction(SD_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(SD_CS, 1);

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_MOSI,
        .miso_io_num = SD_MISO,
        .sclk_io_num = SD_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000
    };
    spi_device_handle_t spi;
    spi_bus_initialize(HSPI_HOST, &bus_cfg, 1);
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 16000000,
        .mode = 0,
        .spics_io_num = SD_CS,
        .queue_size = 1,
    };
    spi_bus_add_device(HSPI_HOST, &dev_cfg, &spi);

    // Mount SD Card
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_miso = SD_MISO;
    slot_config.gpio_mosi = SD_MOSI;
    slot_config.gpio_sck  = SD_SCK;
    slot_config.gpio_cs   = SD_CS;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5
    };
    sdmmc_card_t *card;
    esp_err_t ret_sd = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if(ret_sd != ESP_OK) {
        printf("Failed to mount SD card\n");
        return;
    }

    // Initialize Audio
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(9); // 0...21

    // Initialize GUI
    hal_screen_startup_out();

    // Fill screen with black
    hal_paint_screen_fill(COLOR_BLACK);

    // Play startup sound
    sound_play("/0018_battery_pickup.mp3");

    // Initialize DSP
    fd_dsp_draw_init(&fd_data);

#if UART_PROTOCOL_MAVLINK
    // Initialize MAVLink if needed
#else
    // Initialize parsers
    modParser_init(&parser_rx, PIK_ADR_DEFAULT, PARSER_MAX_DATA_SIZE);
    modParser_init(&parser_tx, PIK_ADR_DEFAULT, PARSER_MAX_DATA_SIZE);
#endif

    printf("\nINIT ALL OK!\n");

    // Create FreeRTOS Tasks
    xTaskCreatePinnedToCore(&mainfunc, "mainfunc", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(&drawfunc, "drawfunc", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(&sendMAVLink, "sendMAVLink", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(&fd_dsp_MAVLink_receive, "fd_dsp_MAVLink_receive", 8192, NULL, 5, NULL, 1);
}

// MAVLink Send Function
void sendMAVLink(void *pvParameters) {
    while(1) {
        fd_dsp_calibrate_send_process();

        static U32 lastSent = 0;
        if((millis() - lastSent) >= 1000) { // Send every second
            lastSent = millis();

            // Generate HEARTBEAT message
            mavlink_message_t msg;
            uint8_t buf[MAVLINK_MAX_PACKET_LEN];

            mavlink_msg_heartbeat_pack(1, MAV_COMP_ID_AUTOPILOT1, &msg, MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, MAV_MODE_FLAG_MANUAL_INPUT_ENABLED, 0, MAV_STATE_STANDBY);
            uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);

            // Send via UART
            uart_write_bytes(UART_NUM_1, (const char*)buf, len);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Adjust delay as needed
    }
}

// MAVLink Receive Function
void fd_dsp_MAVLink_receive(void *pvParameters) {
    mavlink_message_t msg;
    mavlink_status_t status;

    while(1) {
        while(uart_read_bytes(UART_NUM_1, (uint8_t*)&msg, 1, 100 / portTICK_PERIOD_MS) > 0) {
            if(mavlink_parse_char(MAVLINK_COMM_0, msg.msgid, &msg, &status)) {
                // Handle MAVLink messages
                if(msg.msgid == MAVLINK_MSG_ID_FD) {
                    mavlink_fd_t mav_fd;
                    mavlink_msg_fd_decode(&msg, &mav_fd);

                    fsd.version = mav_fd.version;
                    fsd.error_code = mav_fd.error_code;
                    fsd.inc_cnt = mav_fd.inc_cnt;
                    fsd.idnum = mav_fd.idnum;

                    // RSSI after first cascade
                    fd_data.v[0] = mav_fd.b1ch3;
                    fd_data.v[1] = mav_fd.b1ch1;
                    fd_data.v[2] = mav_fd.b1ch2;
                    fd_data.v[3] = mav_fd.b1ch4;

                    // RSSI after second cascade
                    fd_data.v[4] = mav_fd.b2ch3;
                    fd_data.v[5] = mav_fd.b2ch1;
                    fd_data.v[6] = mav_fd.b2ch2;
                    fd_data.v[7] = mav_fd.b2ch4;

                    // SNR levels after cascades
                    fd_data.snr[2] = mav_fd.snr_b1ch1;
                    fd_data.snr[3] = mav_fd.snr_b1ch2;
                    fd_data.snr[4] = mav_fd.snr_b1ch3;
                    fd_data.snr[5] = mav_fd.snr_b1ch4;

                    fd_data.snr[2] = mav_fd.snr_b2ch1;
                    fd_data.snr[3] = mav_fd.snr_b1ch2;
                    fd_data.snr[4] = mav_fd.snr_b2ch3;
                    fd_data.snr[5] = mav_fd.snr_b2ch4;

                    fd_data.fx[0] = mav_fd.x[0];
                    fd_data.fx[1] = mav_fd.x[1];
                    fd_data.fy[0] = mav_fd.y[0];
                    fd_data.fy[1] = mav_fd.y[1];

                    fd_dsp_vector_get(&fd_data, AREA_SIZE * mul);
                    fd_data.fx[0] = fd_data.x[0];
                    fd_data.fx[1] = fd_data.x[1];
                    fd_data.fy[0] = fd_data.y[0];
                    fd_data.fy[1] = fd_data.y[1];

                    fd_dsp_add_new_vector(&fd_data);
                    fd_dsp_calibrate_process(&fd_data, &calibrate_buf[0]);

                    mavlink_msg_cnt++;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Adjust delay as needed
    }
}

// Main Function for Statistics
static void mainfunc(void *pvParameters) {
    static U32 sec = 0;
    static U32 psec = 0;
    while(1) {
        sec = millis() / 1000;
        if(psec != sec) {
            psec = sec;
            fps = frame_count;
            frame_count = 0;
            mavps = mavlink_msg_cnt;
            mavlink_msg_cnt = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Time Function Replacement
U32 millis(void) {
    return (U32)(esp_timer_get_time() / 1000);
}

// HAL Paint Functions
extern "C" {

// Adapted HAL functions for LVGL

COORD hal_paint_screen_width(void) {
    return 800; // LV_HOR_RES_MAX
}

COORD hal_paint_screen_height(void) {
    return 480; // LV_VER_RES_MAX
}

RET hal_paint_init(INT mode) {
    // LVGL typically does not require separate initialization for painting
    return RET_OK;
}

void hal_paint_pixel_set(COORD x, COORD y, COLOR color) {
    // Set pixel on Canvas
    lv_color_t lv_color = lv_color_make((color >> 11) & 0x1F, (color >> 5) & 0x3F, color & 0x1F);
    lv_canvas_set_px(canvas, x, y, lv_color);
}

COLOR hal_paint_pixel_get(COORD x, COORD y) {
    // Get pixel from Canvas
    if((x >= hal_paint_screen_width()) ||
       (y >= hal_paint_screen_height()) ||
       (x < 0) || (y < 0)) {
        return COLOR_BLACK;
    } else {
        lv_color_t lv_color = lv_canvas_get_px(canvas, x, y);
        return ((lv_color.ch.red << 11) | (lv_color.ch.green << 5) | (lv_color.ch.blue));
    }
}

void hal_paint_block(COORD x, COORD y, COORD w, COORD h, COLOR *color) {
    for(COORD j = 0; j < h; j++) {
        for(COORD i = 0; i < w; i++) {
            hal_paint_pixel_set(i + x, j + y, *color++);
        }
    }
}

void hal_paint_block_color(COORD x, COORD y, COORD w, COORD h, COLOR color) {
    // Fill rectangle on Canvas
    lv_color_t lv_color = lv_color_make((color >> 11) & 0x1F, (color >> 5) & 0x3F, color & 0x1F);
    for(COORD j = 0; j < h; j++) {
        for(COORD i = 0; i < w; i++) {
            hal_paint_pixel_set(x + i, y + j, lv_color.full);
        }
    }
}

void hal_paint_block_transparent(COORD x, COORD y, COORD w, COORD h, COLOR *color, COLOR transparent) {
    for(COORD j = 0; j < h; j++) {
        for(COORD i = 0; i < w; i++) {
            if(*color != transparent) {
                hal_paint_pixel_set(i + x, j + y, *color);
            }
            color++;
        }
    }
}

void hal_paint_screen_fill(COLOR color) {
    // Fill entire Canvas with color
    lv_color_t lv_color = lv_color_make((color >> 11) & 0x1F, (color >> 5) & 0x3F, color & 0x1F);
    lv_canvas_fill_bg(canvas, lv_color, LV_OPA_COVER);
}

RET hal_paint_screen_update(void) {
    // Update Canvas on display
    lv_obj_invalidate(canvas); // Mark Canvas for refresh
    return RET_OK;
}

} // extern "C"
