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

#include "c_library_v2/common/mavlink.h"
#include "conf.h"
#include "mavlink_msg_fd.h"
#include "mavlink_msg_fd_clb.h"

#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_lcd_touch_gt911.h"
#include "esp_log.h"

#define SCREEN_W 800
#define SCREEN_H 480
#define DISP_BUF_SIZE (SCREEN_W * 40)
#define UART_NUM UART_NUM_0
#define UART_TX_PIN GPIO_NUM_43
#define UART_RX_PIN GPIO_NUM_44
#define UART_BUF_SIZE 4096

#define LCD_LEDA_GPIO GPIO_NUM_2
#define LCD_LEDK_GPIO GPIO_NUM_3

#define TOUCH_GT911
#define TOUCH_GT911_SCL 20
#define TOUCH_GT911_SDA 19
#define TOUCH_GT911_INT -1
#define TOUCH_GT911_RST -1
#define TOUCH_GT911_ROTATION ROTATION_NORMAL
#define TOUCH_MAP_X1 800
#define TOUCH_MAP_X2 0
#define TOUCH_MAP_Y1 480
#define TOUCH_MAP_Y2 0

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;
static lv_disp_drv_t disp_drv;
static lv_obj_t *canvas = NULL;
static lv_color_t *canvas_buf = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

static uint32_t fps = 0;
static uint32_t frame_count = 0;
U32 err = 0;
CHAR str[256];

#include "_filter.h"
filter_window_FLOAT_t filt_0;

parser_t parser_rx;
U8 parser_rx_buf[PARSER_MAX_DATA_SIZE];
parser_t parser_tx;
U8 parser_tx_buf[PARSER_MAX_DATA_SIZE];
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

void sendMAVLink();
void fd_dsp_MAVLink_recive(void *arg);
static void mainfunc(void);

int touch_last_x = 0, touch_last_y = 0;

esp_lcd_touch_handle_t touch_handle = NULL;

void touch_init() {
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_err_t ret = esp_lcd_new_panel_io_i2c(I2C_NUM_0, &tp_io_config, &tp_io_handle);
    if (ret != ESP_OK) return;

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = SCREEN_W,
        .y_max = SCREEN_H,
        .rst_gpio_num = TOUCH_GT911_RST,
        .int_gpio_num = TOUCH_GT911_INT,
        .flags = {.swap_xy = 0, .mirror_x = 0, .mirror_y = 0},
    };
    ret = esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &touch_handle);
    if (ret != ESP_OK) return;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TOUCH_GT911_SDA,
        .scl_io_num = TOUCH_GT911_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

bool touch_has_signal() {
    if (touch_handle == NULL) return false;
    uint16_t touch_x = 0, touch_y = 0, strength = 0;
    uint8_t point_num = 0;
    return esp_lcd_touch_get_coordinates(touch_handle, &touch_x, &touch_y, &strength, &point_num, 1) && point_num > 0;
}

bool touch_touched() {
    if (touch_handle == NULL) return false;
    uint16_t touch_x = 0, touch_y = 0, strength = 0;
    uint8_t point_num = 0;
    bool is_touched = esp_lcd_touch_get_coordinates(touch_handle, &touch_x, &touch_y, &strength, &point_num, 1);
    if (is_touched && point_num > 0) {
        touch_last_x = touch_x;
        touch_last_y = touch_y;
        return true;
    }
    return false;
}

bool touch_released() {
    return !touch_has_signal();
}

int fd_dsp_touch_has_signal(void) {
    return touch_has_signal();
}

int fd_dsp_touch_touched(void) {
    return touch_touched();
}

int fd_dsp_touch_released(void) {
    return touch_released();
}

INT fd_dsp_button_test(button_t *b) {
    INT ret = 0;
    if (touch_last_x >= b->x && touch_last_x <= (b->x + b->w)) {
        if (touch_last_y >= b->y && touch_last_y <= (b->y + b->h)) {
            ret = 1;
        }
    }
    return ret;
}

static FLOAT calibrate_buf[FD_CLB_DOTS * 2];
INT mavlink_send_state = 0;

void configure_backlight(void) {
    gpio_config_t backlight_leda_cfg = {.pin_bit_mask = (1ULL << LCD_LEDA_GPIO), .mode = GPIO_MODE_OUTPUT};
    gpio_config_t backlight_ledk_cfg = {.pin_bit_mask = (1ULL << LCD_LEDK_GPIO), .mode = GPIO_MODE_OUTPUT};
    gpio_config(&backlight_leda_cfg);
    gpio_set_level(LCD_LEDA_GPIO, 1);
    gpio_config(&backlight_ledk_cfg);
    gpio_set_level(LCD_LEDK_GPIO, 1);
}

void fd_dsp_calibrate_reset(void) {
    if (CMD_NOP == mavlink_send_state) mavlink_send_state = CMD_FLASH_ERASE;
}

void fd_dsp_calibrate_write(void) {
    if (CMD_NOP == mavlink_send_state) mavlink_send_state = CMD_FLASH_WRITE;
}

void fd_dsp_calibrate_send_process(void) {
    mavlink_message_t msg;
    mavlink_fd_clb_t mav_fd_clb;
    static INT iter = 0;
    static INT delay_cnt = 0;
    CHAR str_local[32];

    switch (mavlink_send_state) {
        case CMD_NOP:
            iter = 0;
            break;
        case CMD_FLASH_ERASE:
            if (delay_cnt == 0) {
                mav_fd_clb.cmd = CMD_FLASH_ERASE;
                mav_fd_clb.data_num = 0;
                mav_fd_clb.idnum = fsd.idnum;
                mavlink_msg_fd_clb_encode(SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb);
                uart_write_bytes(UART_NUM, (const char *)parser_tx_buf, mavlink_msg_to_send_buffer(parser_tx_buf, &msg));
            }
            if (++delay_cnt == 30) {
                delay_cnt = 0;
                mavlink_send_state = CMD_NOP;
            }
            break;
        case CMD_FLASH_WRITE:
            if (++delay_cnt == 5) {
                delay_cnt = 0;
                if (iter < 360 / 8) {
                    mav_fd_clb.cmd = CMD_FLASH_WRITE;
                    mav_fd_clb.data_num = iter;
                    mav_fd_clb.idnum = fsd.idnum;
                    for (INT j = 0; j < 8; j++) {
                        mav_fd_clb.clb_val[j] = calibrate_buf[iter * 8 + j];
                    }
                    mavlink_msg_fd_clb_encode(SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb);
                    uart_write_bytes(UART_NUM, (const char *)parser_tx_buf, mavlink_msg_to_send_buffer(parser_tx_buf, &msg));
                } else {
                    mavlink_send_state = CMD_FLASH_FINALIZE;
                }
                iter++;
            }
            break;
        case CMD_FLASH_FINALIZE:
            mav_fd_clb.cmd = CMD_FLASH_FINALIZE;
            mav_fd_clb.data_num = 0;
            mav_fd_clb.idnum = fsd.idnum;
            mavlink_msg_fd_clb_encode(SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb);
            uart_write_bytes(UART_NUM, (const char *)parser_tx_buf, mavlink_msg_to_send_buffer(parser_tx_buf, &msg));
            mavlink_send_state = CMD_NOP;
            break;
        default:
            break;
    }
}

void drawfunc(void) {
    static U32 sec = 0, psec = 0;
    sec = esp_timer_get_time() / 50000;
    if (psec != sec) {
        psec = sec;
        paint_screen_clear();
        fd_dsp_draw_all(&fd_data, &fsd, mode, filter_mode, mul);
        fd_dsp_buttons_processor(&mode, &filter_mode, &mul);
        xsprintf(str, "fps:%d m:%u", fps, mavps);
        paint_color_set(COLOR_WHITE);
        paint_text_xy(620, 2, str);
        hal_paint_screen_update();
        frame_count++;
    }
}

void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (const void *)color_p);
    lv_disp_flush_ready(disp_drv);
}

void display_init(void) {
    lv_init();
    buf1 = (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    buf2 = (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DISP_BUF_SIZE);
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = SCREEN_W;
    disp_drv.ver_res = SCREEN_H;
    lv_disp_drv_register(&disp_drv);
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16,
        .psram_trans_align = 64,
        .num_fbs = 1,
        .clk_src = LCD_CLK_SRC_PLL160M,
        .disp_gpio_num = -1,
        .pclk_gpio_num = UART_TX_PIN,
        .vsync_gpio_num = GPIO_NUM_40,
        .hsync_gpio_num = GPIO_NUM_39,
        .de_gpio_num = GPIO_NUM_41,
        .data_gpio_nums = {
            GPIO_NUM_45, GPIO_NUM_48, GPIO_NUM_47, GPIO_NUM_21, GPIO_NUM_14, GPIO_NUM_1, GPIO_NUM_16, GPIO_NUM_8, GPIO_NUM_3, GPIO_NUM_46,
            GPIO_NUM_9, GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6, GPIO_NUM_7, GPIO_NUM_15
        },
        .timings = {.pclk_hz = 14000000, .h_res = SCREEN_W, .v_res = SCREEN_H, .hsync_back_porch = 40, .hsync_front_porch = 40, .hsync_pulse_width = 48,
                    .vsync_back_porch = 13, .vsync_front_porch = 1, .vsync_pulse_width = 31, .flags.pclk_active_neg = true},
        .flags = {.fb_in_psram = true}};
    esp_lcd_new_rgb_panel(&panel_config, &panel_handle);
}

void uart_init(void) {
    const uart_config_t uart_config_struct = {.baud_rate = 57600, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE,
                                              .stop_bits = UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, .source_clk = UART_SCLK_APB};
    uart_driver_install(UART_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config_struct);
    uart_set_pin(UART_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void lvgl_task(void *pvParameters) {
    while (1) {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void sendMAVLink(void) {
    static uint64_t lastSent = 0;
    uint64_t now = esp_timer_get_time() / 1000;
    if (now - lastSent < 1000) return;
    lastSent = now;
    fd_dsp_calibrate_send_process();
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_heartbeat_pack(1, MAV_COMP_ID_AUTOPILOT1, &msg, MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, MAV_MODE_FLAG_MANUAL_INPUT_ENABLED, 0, MAV_STATE_STANDBY);
    uart_write_bytes(UART_NUM, (const char *)buf, mavlink_msg_to_send_buffer(buf, &msg));
}

void fd_dsp_MAVLink_recive(void *arg) {
    static mavlink_message_t msg;
    static mavlink_status_t status;
    uint8_t data[UART_BUF_SIZE];
    int len;
    while (1) {
        len = uart_read_bytes(UART_NUM, data, UART_BUF_SIZE, pdMS_TO_TICKS(20));
        if (len > 0) {
            for (int i = 0; i < len; i++) {
                uint8_t tmp_char = data[i];
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
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void mainfunc(void) {
    static U32 sec = 0, psec = 0;
    sec = esp_timer_get_time() / 1000;
    if (psec != sec) {
        psec = sec;
        fps = frame_count;
        frame_count = 0;
        mavps = mavlink_msg_cnt;
        mavlink_msg_cnt = 0;
    }
}

void app_main(void) {
    uart_init();
    configure_backlight();
    display_init();
    touch_init();
    canvas_buf = (lv_color_t *)heap_caps_malloc(SCREEN_W * SCREEN_H * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    if (!canvas_buf) return;
    canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, canvas_buf, SCREEN_W, SCREEN_H, LV_IMG_CF_TRUE_COLOR);
    hal_screen_startup_out();
    paint_color_set(COLOR_WHITE);
    paint_color_background_set(COLOR_BLACK);
    paint_font(PAINT_FONT_Generic8, PAINT_FONT_MS);
    paint_font_mastab_set(2);
    paint_font_mode_transparent_set(1);
    fd_dsp_draw_init(&fd_data);
    xTaskCreate(lvgl_task, "lvgl_task", 4096, NULL, 10, NULL);
    xTaskCreate(fd_dsp_MAVLink_recive, "MAVLink_Recive_Task", 4096, NULL, 10, NULL);
    while (1) {
        drawfunc();
        sendMAVLink();
        mainfunc();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

COORD hal_paint_screen_width(void) {
    return SCREEN_W;
}

COORD hal_paint_screen_height(void) {
    return SCREEN_H;
}

RET hal_paint_init(INT mode) {
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
    return RET_OK;
}

void hal_paint_pixel_set(COORD x, COORD y, COLOR color) {
    lv_color_t lv_color;
    lv_color.full = color;
    lv_canvas_set_px(canvas, x, y, lv_color);
}

COLOR hal_paint_pixel_get(COORD x, COORD y) {
    if ((x >= hal_paint_screen_width()) || (y >= hal_paint_screen_height()) || (x < 0) || (y < 0)) {
        return COLOR_BLACK;
    } else {
        lv_color_t lv_color = lv_canvas_get_px(canvas, x, y);
        return lv_color.full;
    }
}

void hal_paint_block(COORD x, COORD y, COORD w, COORD h, COLOR *color) {
    COORD i, j;
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            hal_paint_pixel_set(i + x, j + y, *color++);
        }
    }
}

void hal_paint_block_color(COORD x, COORD y, COORD w, COORD h, COLOR color) {
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color.full = color;
    rect_dsc.bg_opa = LV_OPA_COVER;
    lv_canvas_draw_rect(canvas, x, y, w, h, &rect_dsc);
}

void hal_paint_block_transparent(COORD x, COORD y, COORD w, COORD h, COLOR *color, COLOR transparent) {
    COORD i, j;
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            if (*color != transparent) {
                hal_paint_pixel_set(i + x, j + y, *color);
            }
            color++;
        }
    }
}

void hal_paint_screen_fill(COLOR color) {
    lv_canvas_fill_bg(canvas, lv_color_make((color >> 11) & 0x1F, (color >> 5) & 0x3F, color & 0x1F), LV_OPA_COVER);
}

RET hal_paint_screen_update(void) {
    return RET_OK;
}
