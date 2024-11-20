// Include necessary headers
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/uart.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"

#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch_gt911.h"

// MAVLink headers
#include "c_library_v2/common/mavlink.h"
#include "mavlink_msg_fd.h"
#include "mavlink_msg_fd_clb.h"

// Include your application headers
#include "fd_dsp.h"
#include "modParser.h"
#include "halPaint.h"
#include "xprintf.h"
#include "conf.h"

// Forward declarations of functions
void sendMAVLink(void);
void fd_dsp_MAVLink_recive(void);
void drawfunc(void);
void fd_dsp_calibrate_send_process(void);

// Define constants and variables
static const char *TAG = "MAVLink";

static SemaphoreHandle_t lvgl_mux = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

static lv_obj_t *canvas;
static lv_color_t *canvas_buffer;

static uint32_t fps = 0;
static uint32_t frame_count = 0;
static uint32_t mavps = 0;
uint32_t mavlink_msg_cnt = 0;
uint32_t serial_chars_cnt = 0;

parser_t parser_rx;
uint8_t parser_rx_buf[PARSER_MAX_DATA_SIZE];
parser_t parser_tx;
uint8_t parser_tx_buf[PARSER_MAX_DATA_SIZE];

fd_data_t fd_data;
fd_serial_data_t fsd;
INT mavlink_send_state = 0;  // Ensure this is defined only once
static float calibrate_buf[FD_CLB_DOTS * 2];

#define LCD_LEDA_GPIO GPIO_NUM_2
#define LCD_LEDK_GPIO GPIO_NUM_3

#define TOUCH_GT911_SCL 20
#define TOUCH_GT911_SDA 19

#define MAVLINK_UART_NUM UART_NUM_0
#define TXD_PIN (GPIO_NUM_43)
#define RXD_PIN (GPIO_NUM_44)
#define MAVLINK_BUF_SIZE 256

#define LV_SCREEN_WIDTH  800
#define LV_SCREEN_HEIGHT 480

CHAR str[256];

#include "_filter.h"
filter_window_FLOAT_t filt_0;

U32 ver = 20240407;
U32 mode = 3;
U32 filter_mode = 1;
U32 mul = 1;
FLOAT angle = 0.;

// Function prototypes for internal functions
static void lvgl_tick_inc_cb(void *arg);
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
// static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);
static void lvgl_task(void *arg);
static bool display_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data);

void configure_backlight(void) {
    gpio_config_t backlight_leda_cfg = {
        .pin_bit_mask = (1ULL << LCD_LEDA_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&backlight_leda_cfg));
    gpio_set_level(LCD_LEDA_GPIO, 1);

    gpio_config_t backlight_ledk_cfg = {
        .pin_bit_mask = (1ULL << LCD_LEDK_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&backlight_ledk_cfg));
    gpio_set_level(LCD_LEDK_GPIO, 1);
}

void display_init(void) {
    ESP_LOGI(TAG, "Initializing display");

    configure_backlight();

    // Initialize I2C for touch controller
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TOUCH_GT911_SDA,
        .scl_io_num = TOUCH_GT911_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    ESP_LOGI(TAG, "Configuring I2C for touch controller...");
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0));
    ESP_LOGI(TAG, "I2C initialized successfully for touch controller");

    // Initialize the display
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16,
        .psram_trans_align = 64,
        .num_fbs = 1,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num = -1, // Not used
        .pclk_gpio_num = GPIO_NUM_0,
        .vsync_gpio_num = GPIO_NUM_40,
        .hsync_gpio_num = GPIO_NUM_39,
        .de_gpio_num = GPIO_NUM_41,
        .data_gpio_nums = {
            GPIO_NUM_15, GPIO_NUM_7, GPIO_NUM_6, GPIO_NUM_5, GPIO_NUM_4,
            GPIO_NUM_9, GPIO_NUM_46, GPIO_NUM_3, GPIO_NUM_8, GPIO_NUM_16,
            GPIO_NUM_1, GPIO_NUM_14, GPIO_NUM_21, GPIO_NUM_47, GPIO_NUM_48,
            GPIO_NUM_45
        },
        .timings = {
            .pclk_hz = 14000000,
            .h_res = LV_SCREEN_WIDTH,
            .v_res = LV_SCREEN_HEIGHT,
            .hsync_back_porch = 40,
            .hsync_front_porch = 40,
            .hsync_pulse_width = 48,
            .vsync_back_porch = 13,
            .vsync_front_porch = 1,
            .vsync_pulse_width = 31,
            .flags.pclk_active_neg = 1,
        },
        .flags = {
            .fb_in_psram = true,
        }
    };
    ESP_LOGI(TAG, "Creating RGB panel...");
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));
    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_vsync = display_on_vsync_event,
    };
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, NULL));
    ESP_LOGI(TAG, "Resetting and initializing RGB panel...");
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    // Initialize touch controller GT911
    ESP_LOGI(TAG, "Initializing touch controller GT911");
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_err_t ret = esp_lcd_new_panel_io_i2c(I2C_NUM_0, &tp_io_config, &tp_io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C panel IO for GT911: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "I2C panel IO for GT911 created successfully");
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = LV_SCREEN_WIDTH,
        .y_max = LV_SCREEN_HEIGHT,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    ESP_LOGI(TAG, "Creating touch handle for GT911...");
    ret = esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &touch_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize GT911 touch controller: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "GT911 touch controller initialized successfully");

    // Initialize LVGL
    ESP_LOGI(TAG, "Initializing LVGL library");
    lv_init();

    // Allocate draw buffers
    static lv_disp_draw_buf_t draw_buf;
    lv_color_t *buf1 = heap_caps_malloc(LV_SCREEN_WIDTH * 100 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    lv_color_t *buf2 = heap_caps_malloc(LV_SCREEN_WIDTH * 100 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 && buf2);
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LV_SCREEN_WIDTH * 100);

    // Register display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LV_SCREEN_WIDTH;
    disp_drv.ver_res = LV_SCREEN_HEIGHT;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_drv_register(&disp_drv);

    // Register touch input device
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    // indev_drv.read_cb = lvgl_touch_cb;
    indev_drv.user_data = touch_handle;

    lv_indev_drv_register(&indev_drv);

    // Install LVGL tick timer
    ESP_LOGI(TAG, "Installing LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_tick_inc_cb,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, 10 * 1000));

    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    assert(lvgl_mux);

    // Create LVGL task
    ESP_LOGI(TAG, "Creating LVGL task");
    xTaskCreate(lvgl_task, "lvgl_task", 4096, NULL, 5, NULL);

    // Initialize canvas
    canvas_buffer = heap_caps_malloc(LV_SCREEN_WIDTH * LV_SCREEN_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(canvas_buffer);

    canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, canvas_buffer, LV_SCREEN_WIDTH, LV_SCREEN_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
}

static bool display_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data) {
    return false;
}

static void lvgl_tick_inc_cb(void *arg) {
    lv_tick_inc(10);
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

// static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
// {
//     uint16_t touchpad_x[1] = {0};
//     uint16_t touchpad_y[1] = {0};
//     uint8_t touchpad_cnt = 0;

//     esp_lcd_touch_read_data(drv->user_data);

//     bool touchpad_pressed = esp_lcd_touch_get_coordinates(
//         drv->user_data, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);
//     if (touchpad_pressed && touchpad_cnt > 0) {
//         data->point.x = touchpad_x[0];
//         data->point.y = touchpad_y[0];
//         data->state = LV_INDEV_STATE_PRESSED;
//         ESP_LOGD(TAG, "Touch detected at (%u, %u) with count %u", touchpad_x[0], touchpad_y[0], touchpad_cnt);
//     } else {
//         data->state = LV_INDEV_STATE_RELEASED;
//         ESP_LOGD(TAG, "No touch detected");
//     }
// }

static void lvgl_task(void *arg) {
    while (1) {
        if (xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY) == pdTRUE) {
            lv_timer_handler();
            xSemaphoreGiveRecursive(lvgl_mux);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void uart_init() {
    const uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(MAVLINK_UART_NUM, 4096, 0, 0, NULL, 0);
    uart_param_config(MAVLINK_UART_NUM, &uart_config);
    uart_set_pin(MAVLINK_UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}


void sendMAVLink_task(void *arg) {
    while (1) {
        sendMAVLink();
        vTaskDelay(pdMS_TO_TICKS(1000)); // Send every second
    }
}

void receiveMAVLink_task(void *arg) {
    while (1) {
        fd_dsp_MAVLink_recive();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void draw_task(void *arg) {
    while (1) {
        if (xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY) == pdTRUE) {
            drawfunc();
            xSemaphoreGiveRecursive(lvgl_mux);
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Adjust the delay as needed
    }
}

static FLOAT calibrate_buf[FD_CLB_DOTS * 2];
void fd_dsp_calibrate_reset(void) {
    if (CMD_NOP == mavlink_send_state)
        mavlink_send_state = CMD_FLASH_ERASE;
}

void fd_dsp_calibrate_write(void) {
    if (CMD_NOP == mavlink_send_state)
        mavlink_send_state = CMD_FLASH_WRITE;
}

void fd_dsp_calibrate_send_process(void) {
    mavlink_message_t msg;
    mavlink_fd_clb_t mav_fd_clb;
    static int iter = 0;
    static int delay = 0;
    char str[32];

    switch (mavlink_send_state) {
        case CMD_NOP:
            iter = 0;
            break;

        case CMD_FLASH_ERASE:
            xsprintf(str, "Sending erase...");
            paint_color_set(COLOR_ORANGE);
            paint_text_xy(310, 20, str);

            if (0 == delay) {
                mav_fd_clb.cmd = CMD_FLASH_ERASE;
                mav_fd_clb.data_num = 0;
                mav_fd_clb.idnum = fsd.idnum;

                mavlink_msg_fd_clb_encode(SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb);
                int size = mavlink_msg_to_send_buffer(parser_tx_buf, &msg);

                uart_write_bytes(MAVLINK_UART_NUM, (const char *)parser_tx_buf, size);
            }
            delay++;
            if (30 == delay) {
                delay = 0;
                mavlink_send_state = CMD_NOP;
            }
            break;

        case CMD_FLASH_WRITE:
            xsprintf(str, "Sending %u...", iter);
            paint_color_set(COLOR_ORANGE);
            paint_text_xy(310, 20, str);

            delay++;
            if (5 == delay) {
                delay = 0;
                if (iter < FD_CLB_DOTS / 8) {
                    mav_fd_clb.cmd = CMD_FLASH_WRITE;
                    mav_fd_clb.data_num = iter;
                    mav_fd_clb.idnum = fsd.idnum;

                    for (int j = 0; j < 8; j++) {
                        mav_fd_clb.clb_val[j] = calibrate_buf[iter * 8 + j];
                    }

                    mavlink_msg_fd_clb_encode(SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb);
                    int size = mavlink_msg_to_send_buffer(parser_tx_buf, &msg);

                    uart_write_bytes(MAVLINK_UART_NUM, (const char *)parser_tx_buf, size);
                } else {
                    mavlink_send_state = CMD_FLASH_FINALIZE;
                }
                iter++;
            }
            break;

        case CMD_FLASH_FINALIZE:
            xsprintf(str, "Sending finished...");
            paint_color_set(COLOR_ORANGE);
            paint_text_xy(310, 20, str);

            mav_fd_clb.cmd = CMD_FLASH_FINALIZE;
            mav_fd_clb.data_num = 0;
            mav_fd_clb.idnum = fsd.idnum;

            mavlink_msg_fd_clb_encode(SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb);
            int size = mavlink_msg_to_send_buffer(parser_tx_buf, &msg);

            uart_write_bytes(MAVLINK_UART_NUM, (const char *)parser_tx_buf, size);

            mavlink_send_state = CMD_NOP;
            break;

        default:
            break;
    }
}

void sendMAVLink(void) {
    fd_dsp_calibrate_send_process();

    static uint32_t lastSent = 0;
    uint32_t current_time = lv_tick_get();
    if (current_time - lastSent < 1000)
        return;

    lastSent = current_time;

    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    mavlink_msg_heartbeat_pack(
        1, MAV_COMP_ID_AUTOPILOT1, &msg,
        MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC,
        MAV_MODE_FLAG_MANUAL_INPUT_ENABLED, 0, MAV_STATE_STANDBY
    );

    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);

    int written = uart_write_bytes(MAVLINK_UART_NUM, (const char *)buf, len);
    if (written != len) {
        ESP_LOGE("MAVLink", "Failed to write MAVLink message to UART");
    }
}

void fd_dsp_MAVLink_recive(void) {
    static mavlink_message_t msg;
    static mavlink_status_t status;

    uint8_t tmp_char;
    size_t buffered_size;
    uint32_t cnt_max = 4000;

    ESP_ERROR_CHECK(uart_get_buffered_data_len(MAVLINK_UART_NUM, &buffered_size));

    while (buffered_size > 0 && cnt_max--) {
        int len = uart_read_bytes(MAVLINK_UART_NUM, &tmp_char, 1, pdMS_TO_TICKS(10));
            serial_chars_cnt++;

#if UART_PROTOCOL_MAVLINK
            if (mavlink_parse_char(MAVLINK_COMM_0, tmp_char, &msg, &status)) {
                if (MAVLINK_MSG_ID_FD == msg.msgid) {
                    mavlink_fd_t mav_fd = {0};
                    mavlink_msg_fd_decode(&msg, &mav_fd);

                    fsd.version = mav_fd.version;
                    fsd.error_code = mav_fd.error_code;
                    fsd.inc_cnt = mav_fd.inc_cnt;
                    fsd.idnum = mav_fd.idnum;

                    fd_data.v[0] = mav_fd.b1ch3;
                    fd_data.v[1] = mav_fd.b1ch1;
                    fd_data.v[2] = mav_fd.b1ch2;
                    fd_data.v[3] = mav_fd.b1ch4;

                    fd_data.v[4] = mav_fd.b2ch3;
                    fd_data.v[5] = mav_fd.b2ch1;
                    fd_data.v[6] = mav_fd.b2ch2;
                    fd_data.v[7] = mav_fd.b2ch4;

                    fd_data.snr[2] = mav_fd.snr_b1ch1;
                    fd_data.snr[3] = mav_fd.snr_b1ch2;
                    fd_data.snr[4] = mav_fd.snr_b1ch3;
                    fd_data.snr[5] = mav_fd.snr_b1ch4;

                    fd_data.snr[2] = mav_fd.snr_b2ch1;
                    fd_data.snr[3] = mav_fd.snr_b2ch2;
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
#else
            int size;
            if (RET_OK == modParser_decode(&parser_rx, tmp_char, parser_rx_buf, &size)) {
                if (0 == fd_dsp_rx_packet((uint8_t *)parser_rx_buf, &fsd, &fd_data)) {
                    fd_dsp_add_new_vector(&fd_data, mul);
                    mavlink_msg_cnt++;
                }
            }
#endif
            // Update buffered_size
            ESP_ERROR_CHECK(uart_get_buffered_data_len(MAVLINK_UART_NUM, &buffered_size));
        }
    }

void drawfunc(void) {
    static uint32_t last_time = 0;
    uint32_t current_time = lv_tick_get();
    if (current_time - last_time >= 50) { // Adjust the refresh rate as needed
        last_time = current_time;

        paint_screen_clear();

        fd_dsp_draw_all(&fd_data, &fsd, mode, filter_mode, mul);
        // fd_dsp_buttons_processor(&mode, &filter_mode, &mul);

        xsprintf(str, "fps:%d m:%u", fps, mavps);
        paint_color_set(COLOR_WHITE);
        paint_text_xy(620, 2, str);

        hal_paint_screen_update();
        frame_count++;
    }
}

void app_main(void) {
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    display_init();
    uart_init();

    fd_dsp_draw_init(&fd_data);

    xTaskCreate(draw_task, "draw_task", 8192, NULL, 5, NULL);
    xTaskCreate(sendMAVLink_task, "sendMAVLink_task", 4096, NULL, 5, NULL);
    xTaskCreate(receiveMAVLink_task, "receiveMAVLink_task", 4096, NULL, 5, NULL);

    // The main task can be suspended or used for other initialization
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

COORD hal_paint_screen_width(void) {
    return LV_SCREEN_WIDTH;
}

COORD hal_paint_screen_height(void) {
    return LV_SCREEN_HEIGHT;
}

RET hal_paint_init(INT mode) {
    // Initialization done in display_init()
    return RET_OK;
}

void hal_paint_pixel_set(COORD x, COORD y, COLOR color) {
    if (x >= 0 && x < LV_SCREEN_WIDTH && y >= 0 && y < LV_SCREEN_HEIGHT) {
        lv_color_t lv_color = lv_color_hex(color);
        lv_canvas_set_px(canvas, x, y, lv_color);
    }
}

COLOR hal_paint_pixel_get(COORD x, COORD y) {
    if (x >= 0 && x < LV_SCREEN_WIDTH && y >= 0 && y < LV_SCREEN_HEIGHT) {
        lv_color_t lv_color = lv_canvas_get_px(canvas, x, y);
        return lv_color_to32(lv_color);
    }
    return COLOR_BLACK;
}

void hal_paint_block(COORD x, COORD y, COORD w, COORD h, COLOR *color) {
    for (COORD j = 0; j < h; j++) {
        for (COORD i = 0; i < w; i++) {
            hal_paint_pixel_set(x + i, y + j, *color++);
        }
    }
}

void hal_paint_block_color(COORD x, COORD y, COORD w, COORD h, COLOR color) {
    lv_color_t lv_color = lv_color_hex(color);
    lv_area_t area = { x, y, x + w - 1, y + h - 1 };
    lv_canvas_fill_bg(canvas, lv_color, LV_OPA_COVER);
}

void hal_paint_block_transparent(COORD x, COORD y, COORD w, COORD h, COLOR *color, COLOR transparent) {
    for (COORD j = 0; j < h; j++) {
        for (COORD i = 0; i < w; i++) {
            if (*color != transparent) {
                hal_paint_pixel_set(x + i, y + j, *color);
            }
            color++;
        }
    }
}

void hal_paint_screen_fill(COLOR color) {
    lv_color_t lv_color = lv_color_hex(color);
    lv_canvas_fill_bg(canvas, lv_color, LV_OPA_COVER);
}

RET hal_paint_screen_update(void) {
    // Invalidate the canvas to trigger a redraw
    lv_obj_invalidate(canvas);
    return RET_OK;
}
