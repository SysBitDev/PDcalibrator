#include <stdio.h>
#include <stdint.h>      // For uint32_t
#include <inttypes.h>    // For PRIu32
#include "sdkconfig.h"
#include "lvgl.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_lcd_touch_gt911.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include <c_library_v2/common/mavlink.h>
#include "mavlink_msg_fd.h"
#include "mavlink_msg_fd_clb.h"
#include "conf.h"
#include "driver/uart.h"
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// Logging Tag
static const char *TAG = "display";

// Type Definitions
typedef struct {
    uint32_t version;
    uint8_t calibrated;
    uint8_t error_code;
    uint8_t ch_num;
    uint8_t branches_num;
    uint8_t vector_mode;
    uint8_t inc_cnt;
    uint8_t idnum;
} fd_serial_data_t;

typedef struct {
    uint32_t v[8];
    uint32_t snr[8];
    float x[2];
    float y[2];
    float dir[2];
    float len[2];
    float fx[2];
    float fy[2];
    float fpx[2];
    float fpy[2];
    uint8_t fd_num;
    uint16_t buf_size;
} fd_data_t;

// Global Variables
static SemaphoreHandle_t lvgl_mux = NULL;
static SemaphoreHandle_t fd_data_mutex = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

#define LCD_LEDA_GPIO GPIO_NUM_2
#define LCD_LEDK_GPIO GPIO_NUM_3

#define TOUCH_GT911_SCL 20
#define TOUCH_GT911_SDA 19

#define UART_NUM         UART_NUM_1
#define BUF_SIZE         (1024)
#define TXD_PIN          GPIO_NUM_17
#define RXD_PIN          GPIO_NUM_16
#define BAUD_RATE        57600

#define AREA_SIZE 100 // Adjust as needed
#define VECTOR_HISTORY_SIZE 128

static uint8_t tx_buffer[MAVLINK_MAX_PACKET_LEN];
static uint8_t rx_buffer[BUF_SIZE];

// Declare fsd and fd_data as global variables
static fd_serial_data_t fsd;
static fd_data_t fd_data;

// Declare vector history
static int vector_history_x[VECTOR_HISTORY_SIZE];
static int vector_history_y[VECTOR_HISTORY_SIZE];

// Declare mavlink_msg_cnt globally
static uint32_t mavlink_msg_cnt = 0;

// Additional Metrics
static float mul = AREA_SIZE;
static float radius = 0.0f;
static float roughness = 0.0f;

// LVGL objects
// Left Labels
static lv_obj_t *label_x_pd;
static lv_obj_t *label_pd_x_sm;
static lv_obj_t *label_y_pd;
static lv_obj_t *label_pd_y_sm;

// Right Labels
static lv_obj_t *label_ch[8];
static lv_obj_t *label_snr[8];
static lv_obj_t *label_mul;
static lv_obj_t *label_radius;
static lv_obj_t *label_roughness;

// Center Canvas
static lv_obj_t *canvas;
static lv_color_t *canvas_buf = NULL;

// Paint structure and functions
typedef struct {
    lv_color_t fg_color; // Foreground color
    lv_color_t bg_color; // Background color
    const lv_font_t *font;     // Current font (const)
    int font_num;
    int font_mashtab;
    int font_mode_transparent;
    int cX;
    int cY;
    int W; // Font width
    int H; // Font height
    int maxCol;
    int maxRow;
    int offsetX;
    int offsetY;
} paint_t;

static paint_t paint;

// Function Prototypes
static void lvgl_tick_inc_cb(void *arg);
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);
static void lvgl_task(void *arg);
static bool display_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data);
static void lvgl_ui_init(void);
void mavlink_calibrate_send_process(void);
void send_heartbeat(void);
void fd_dsp_vector_get(fd_data_t *fd, float amp);
void fd_dsp_add_new_vector(fd_data_t *pfd_data);
void uart_init(void);
void mavlink_receive_task(void *arg);

// Paint functions adapted to LVGL
void paint_init_custom(void) {
    paint.fg_color = lv_color_white();
    paint.bg_color = lv_color_black();
    paint.font = &lv_font_montserrat_14; // Use default font
    paint.font_num = 0;
    paint.font_mashtab = 1;
    paint.font_mode_transparent = 0;
    
    // Correctly get font width and height
    lv_font_glyph_dsc_t dsc;
    bool got_glyph = lv_font_get_glyph_dsc(paint.font, &dsc, 'M', 0);
    if (got_glyph) {
        paint.W = dsc.box_w;
        paint.H = dsc.box_h;
    } else {
        // Default values if glyph not found
        paint.W = 16;
        paint.H = 16;
    }
    
    paint.cX = 0;
    paint.cY = 0;
    paint.maxCol = lv_obj_get_width(canvas) / paint.W;
    paint.maxRow = lv_obj_get_height(canvas) / paint.H;
    paint.offsetX = 0;
    paint.offsetY = 0;
}

void paint_color_set(lv_color_t color) {
    paint.fg_color = color;
}

void paint_color_background_set(lv_color_t color) {
    paint.bg_color = color;
}

void paint_pixel(int x, int y) {
    lv_canvas_set_px(canvas, x, y, paint.fg_color);
}

void paint_line(int x1, int y1, int x2, int y2) {
    lv_point_t points[2] = {{x1, y1}, {x2, y2}};
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.color = paint.fg_color;
    line_dsc.width = 1;
    lv_canvas_draw_line(canvas, points, 2, &line_dsc);
}

void paint_rectangle(int x, int y, int w, int h, bool filled) {
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    if (filled) {
        rect_dsc.bg_color = paint.fg_color;
        rect_dsc.bg_opa = LV_OPA_COVER;
        rect_dsc.border_width = 0;
    } else {
        rect_dsc.bg_opa = LV_OPA_TRANSP;
        rect_dsc.border_width = 1;
        rect_dsc.border_color = paint.fg_color;
    }
    lv_canvas_draw_rect(canvas, x, y, w, h, &rect_dsc);
}

void paint_circle(int x, int y, int radius, bool filled) {
    if (filled) {
        lv_draw_rect_dsc_t rect_dsc;
        lv_draw_rect_dsc_init(&rect_dsc);
        rect_dsc.bg_color = paint.fg_color;
        rect_dsc.bg_opa = LV_OPA_COVER;
        rect_dsc.radius = radius;
        rect_dsc.border_width = 0;
        lv_canvas_draw_rect(canvas, x - radius, y - radius, 2 * radius, 2 * radius, &rect_dsc);
    } else {
        lv_draw_arc_dsc_t arc_dsc;
        lv_draw_arc_dsc_init(&arc_dsc);
        arc_dsc.color = paint.fg_color;
        arc_dsc.width = 1; // Товщина лінії
        arc_dsc.opa = LV_OPA_COVER;
        uint16_t angle_start = 0;
        uint16_t angle_end = 360;
        lv_canvas_draw_arc(canvas, x, y, radius, angle_start, angle_end, &arc_dsc);
    }
}


void paint_text_xy(int x, int y, const char *str) {
    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = paint.fg_color;
    label_dsc.font = paint.font;
    lv_canvas_draw_text(canvas, x, y, lv_obj_get_width(canvas) - x, &label_dsc, str);
}

void paint_screen_clear(void) {
    lv_canvas_fill_bg(canvas, paint.bg_color, LV_OPA_COVER);
}

// Initialize Backlight
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

// Initialize Display and LVGL
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

    // Configure RGB Panel
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16,
        .psram_trans_align = 64,
        .num_fbs = 1,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num = LCD_LEDA_GPIO,
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
            .pclk_hz = 15000000,
            .h_res = 800,
            .v_res = 480,
            .hsync_back_porch = 88,
            .hsync_front_porch = 40,
            .hsync_pulse_width = 48,
            .vsync_back_porch = 13,
            .vsync_front_porch = 1,
            .vsync_pulse_width = 31,
            .flags.pclk_active_neg = 0,
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

    // Initialize Touch Controller GT911
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
        .x_max = 800,
        .y_max = 480,
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

    static lv_disp_draw_buf_t draw_buf;
    void *buf1 = heap_caps_malloc(800 * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf1);
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, 800 * 100);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 800;
    disp_drv.ver_res = 480;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_cb;
    indev_drv.user_data = touch_handle;

    lv_indev_drv_register(&indev_drv);

    // Install LVGL Tick Timer
    ESP_LOGI(TAG, "Installing LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_tick_inc_cb,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, 10 * 1000));

    // Create Mutexes
    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    assert(lvgl_mux);

    fd_data_mutex = xSemaphoreCreateMutex();
    assert(fd_data_mutex != NULL);

    // Initialize LVGL UI
    lvgl_ui_init();

    // Initialize paint
    paint_init_custom();

    // Create LVGL Task
    ESP_LOGI(TAG, "Creating LVGL task");
    xTaskCreate(lvgl_task, "lvgl_task", 8192, NULL, 5, NULL);
}

// Display VSYNC Event Callback (Not Used)
static bool display_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data) {
    return false;
}

// LVGL Tick Increment Callback
static void lvgl_tick_inc_cb(void *arg) {
    lv_tick_inc(10);
}

// LVGL Flush Callback
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

// LVGL Touch Input Callback
static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    esp_lcd_touch_read_data(drv->user_data);

    bool touchpad_pressed = esp_lcd_touch_get_coordinates(
        drv->user_data, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);
    if (touchpad_pressed && touchpad_cnt > 0) {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PR;
        ESP_LOGD(TAG, "Touch detected at (%d, %d) with count %d", touchpad_x[0], touchpad_y[0], touchpad_cnt);
    } else {
        data->state = LV_INDEV_STATE_REL;
        ESP_LOGD(TAG, "No touch detected");
    }
}

// Initialize LVGL UI Elements
static void lvgl_ui_init(void) {
    // Create a main container with horizontal layout
    lv_obj_t *main_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_container, 800, 480);
    lv_obj_set_style_bg_color(main_container, lv_color_black(), 0);
    lv_obj_set_layout(main_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(main_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(main_container, 10, 0);

    // Left Container
    lv_obj_t *left_container = lv_obj_create(main_container);
    lv_obj_set_size(left_container, 200, 480);
    lv_obj_set_layout(left_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(left_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_gap(left_container, 10, 0);

    // Create Labels on the Left
    label_x_pd = lv_label_create(left_container);
    lv_label_set_text(label_x_pd, "X_PD: 0.00");
    lv_obj_set_style_text_font(label_x_pd, &lv_font_montserrat_16, 0);

    label_pd_x_sm = lv_label_create(left_container);
    lv_label_set_text(label_pd_x_sm, "PD_X_SM: 0.00");
    lv_obj_set_style_text_font(label_pd_x_sm, &lv_font_montserrat_16, 0);

    label_y_pd = lv_label_create(left_container);
    lv_label_set_text(label_y_pd, "Y_PD: 0.00");
    lv_obj_set_style_text_font(label_y_pd, &lv_font_montserrat_16, 0);

    label_pd_y_sm = lv_label_create(left_container);
    lv_label_set_text(label_pd_y_sm, "PD_Y_SM: 0.00");
    lv_obj_set_style_text_font(label_pd_y_sm, &lv_font_montserrat_16, 0);

    // Center Container
    lv_obj_t *center_container = lv_obj_create(main_container);
    lv_obj_set_size(center_container, 400, 480);
    lv_obj_set_layout(center_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(center_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(center_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(center_container, 10, 0);

    // Create a canvas to draw the vector
    canvas = lv_canvas_create(center_container);
    lv_obj_set_size(canvas, 380, 460);
    lv_obj_set_style_bg_color(canvas, lv_color_black(), 0);
    lv_obj_set_style_border_color(canvas, lv_color_white(), 0);
    lv_obj_set_style_border_width(canvas, 2, 0);
    lv_obj_set_style_border_side(canvas, LV_BORDER_SIDE_FULL, 0);

    // Allocate buffer for the canvas
    size_t canvas_size = LV_CANVAS_BUF_SIZE_TRUE_COLOR(380, 460);
    canvas_buf = heap_caps_malloc(canvas_size, MALLOC_CAP_SPIRAM);
    assert(canvas_buf != NULL);
    lv_canvas_set_buffer(canvas, canvas_buf, 380, 460, LV_IMG_CF_TRUE_COLOR);

    // Clear the canvas
    lv_canvas_fill_bg(canvas, lv_color_make(0, 0, 0), LV_OPA_COVER);

    // Right Container
    lv_obj_t *right_container = lv_obj_create(main_container);
    lv_obj_set_size(right_container, 200, 480);
    lv_obj_set_layout(right_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(right_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_gap(right_container, 10, 0);

    // Create Labels on the Right for Channels and SNRs
    for (int i = 0; i < 8; i++) {
        label_ch[i] = lv_label_create(right_container);
        lv_label_set_text_fmt(label_ch[i], "CH %d: 0", i + 1);
        lv_obj_set_style_text_font(label_ch[i], &lv_font_montserrat_14, 0);
    }

    for (int i = 0; i < 8; i++) {
        label_snr[i] = lv_label_create(right_container);
        lv_label_set_text_fmt(label_snr[i], "SNR %d: 0", i + 1);
        lv_obj_set_style_text_font(label_snr[i], &lv_font_montserrat_14, 0);
    }

    // Additional Metrics Labels
    label_mul = lv_label_create(right_container);
    lv_label_set_text(label_mul, "Mul: 1.00");
    lv_obj_set_style_text_font(label_mul, &lv_font_montserrat_14, 0);

    label_radius = lv_label_create(right_container);
    lv_label_set_text(label_radius, "Radius: 0.00");
    lv_obj_set_style_text_font(label_radius, &lv_font_montserrat_14, 0);

    label_roughness = lv_label_create(right_container);
    lv_label_set_text(label_roughness, "Roughness: 0.00");
    lv_obj_set_style_text_font(label_roughness, &lv_font_montserrat_14, 0);
}

// LVGL Task
static void lvgl_task(void *arg) {
    while (1) {
        if (xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY) == pdTRUE) {
            lv_timer_handler();

            // Update labels and canvas
            if (xSemaphoreTake(fd_data_mutex, portMAX_DELAY) == pdTRUE) {
                // Update Left Labels
                lv_label_set_text_fmt(label_x_pd, "X_PD: %.2f", fd_data.x[0]);
                lv_label_set_text_fmt(label_pd_x_sm, "PD_X_SM: %.2f", fd_data.fx[0]);
                lv_label_set_text_fmt(label_y_pd, "Y_PD: %.2f", fd_data.y[0]);
                lv_label_set_text_fmt(label_pd_y_sm, "PD_Y_SM: %.2f", fd_data.fy[0]);

                // Update Right Labels for Channels and SNRs
                for (int i = 0; i < 8; i++) {
                    // Using PRIu32 for uint32_t
                    lv_label_set_text_fmt(label_ch[i], "CH %d: %" PRIu32, i + 1, fd_data.v[i]);
                    lv_label_set_text_fmt(label_snr[i], "SNR %d: %" PRIu32, i + 1, fd_data.snr[i]);
                }

                // Update Additional Metrics
                lv_label_set_text_fmt(label_mul, "Mul: %.2f", mul);
                lv_label_set_text_fmt(label_radius, "Radius: %.2f", radius);
                lv_label_set_text_fmt(label_roughness, "Roughness: %.2f", roughness);

                // Clear the canvas
                paint_screen_clear();

                // Draw the history
                int center_x = 190; // Half of canvas width (380/2)
                int center_y = 230; // Half of canvas height (460/2)

                // Set color to green for history
                paint_color_set(lv_color_make(0, 255, 0));

                for (int i = 0; i < VECTOR_HISTORY_SIZE - 1; i++) {
                    int x1 = center_x + vector_history_x[i];
                    int y1 = center_y + vector_history_y[i];
                    int x2 = center_x + vector_history_x[i+1];
                    int y2 = center_y + vector_history_y[i+1];
                    paint_line(x1, y1, x2, y2);
                }

                // Draw the current vector
                // Set color to red
                paint_color_set(lv_color_make(255, 0, 0));

                int x = center_x + (int)fd_data.x[0];
                int y = center_y + (int)fd_data.y[0];

                paint_line(center_x, center_y, x, y);

                xSemaphoreGive(fd_data_mutex);
            }

            xSemaphoreGiveRecursive(lvgl_mux);
        }
        vTaskDelay(pdMS_TO_TICKS(30)); // Adjust delay as needed
    }
}

// UART Initialization
void uart_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

// Placeholder for Calibration Process (Implement as Needed)
void mavlink_calibrate_send_process(void)
{
    // Implement calibration logic here if needed
}

// Send MAVLink Heartbeat
void send_heartbeat(void)
{
    static uint32_t last_sent = 0;
    if (xTaskGetTickCount() - last_sent < pdMS_TO_TICKS(1000))
        return;

    last_sent = xTaskGetTickCount();

    mavlink_message_t msg;
    mavlink_msg_heartbeat_pack(1, MAV_COMP_ID_AUTOPILOT1, &msg, MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, MAV_MODE_FLAG_MANUAL_INPUT_ENABLED, 0, MAV_STATE_STANDBY);
    int len = mavlink_msg_to_send_buffer(tx_buffer, &msg);
    uart_write_bytes(UART_NUM, (const char *)tx_buffer, len);
}

// Compute Vector from RSSI Values
void fd_dsp_vector_get(fd_data_t *fd, float amp)
{
    float q1, q2, q3, q4, qsum, x, y;
    uint32_t i;

    fd->fd_num = 8; // Assuming we have 8 channels

    switch(fd->fd_num)
    {
        case 8:
            for(i = 0; i < fd->fd_num; i += 4)
            {
                q1 = (float)fd->v[i+0];
                q2 = (float)fd->v[i+1];
                q3 = (float)fd->v[i+2];
                q4 = (float)fd->v[i+3];

                qsum = q1 + q2 + q3 + q4;
                qsum = (qsum < 1) ? 1 : qsum; // Prevent division by zero

                x = (((q1 + q4) - (q2 + q3))) / qsum;
                y = (((q1 + q2) - (q4 + q3))) / qsum;

                x = x * amp;
                y = y * amp;

                fd->x[i/4] = x;
                fd->y[i/4] = y;
            }
            break;

        default:
            break;
    }

    // Calculate additional metrics
    radius = sqrtf(fd->x[0] * fd->x[0] + fd->y[0] * fd->y[0]);
    roughness = fabsf(fd->x[0] - fd->y[0]); // Example calculation
}

// Add New Vector to History
void fd_dsp_add_new_vector(fd_data_t *pfd_data)
{
    // Shift the history
    for(int i = VECTOR_HISTORY_SIZE -1; i > 0; i--)
    {
        vector_history_x[i] = vector_history_x[i-1];
        vector_history_y[i] = vector_history_y[i-1];
    }

    // Add new vector
    vector_history_x[0] = (int)pfd_data->x[0];
    vector_history_y[0] = (int)pfd_data->y[0];
}

// MAVLink Receive Task
void mavlink_receive_task(void *arg)
{
    mavlink_message_t msg;
    mavlink_status_t status;

    while (1)
    {
        int len = uart_read_bytes(UART_NUM, rx_buffer, BUF_SIZE, pdMS_TO_TICKS(10));
        for(int i = 0; i < len; i++)
        {
            if(mavlink_parse_char(MAVLINK_COMM_0, rx_buffer[i], &msg, &status))
            {
                if(msg.msgid == MAVLINK_MSG_ID_FD)
                {
                    mavlink_fd_t mav_fd = {0};
                    mavlink_msg_fd_decode(&msg, &mav_fd);

                    if(xSemaphoreTake(fd_data_mutex, portMAX_DELAY) == pdTRUE) {
                        fsd.version = mav_fd.version;
                        fsd.error_code = mav_fd.error_code;
                        fsd.inc_cnt = mav_fd.inc_cnt;
                        fsd.idnum = mav_fd.idnum;

                        // RSSI after 1-x cascade
                        fd_data.v[0] = mav_fd.b1ch3; // for ch1
                        fd_data.v[1] = mav_fd.b1ch1; // for ch2
                        fd_data.v[2] = mav_fd.b1ch2; // for ch3
                        fd_data.v[3] = mav_fd.b1ch4; // for ch4

                        // RSSI after 2-x cascade
                        fd_data.v[4] = mav_fd.b2ch3; // for ch1
                        fd_data.v[5] = mav_fd.b2ch1; // for ch2
                        fd_data.v[6] = mav_fd.b2ch2; // for ch3
                        fd_data.v[7] = mav_fd.b2ch4; // for ch4

                        // SNR level after 1-x cascade
                        fd_data.snr[0] = mav_fd.snr_b1ch1; // for ch1
                        fd_data.snr[1] = mav_fd.snr_b1ch2; // for ch2
                        fd_data.snr[2] = mav_fd.snr_b1ch3; // for ch3
                        fd_data.snr[3] = mav_fd.snr_b1ch4; // for ch4

                        // SNR level after 2-x cascade
                        fd_data.snr[4] = mav_fd.snr_b2ch1; // for ch1
                        fd_data.snr[5] = mav_fd.snr_b2ch2; // for ch2
                        fd_data.snr[6] = mav_fd.snr_b2ch3; // for ch3
                        fd_data.snr[7] = mav_fd.snr_b2ch4; // for ch4

                        // Compute vector and additional metrics
                        fd_dsp_vector_get(&fd_data, mul); // Adjust AREA_SIZE as needed

                        // Update vector history
                        fd_dsp_add_new_vector(&fd_data);

                        xSemaphoreGive(fd_data_mutex);
                    }

                    // Increment message count
                    mavlink_msg_cnt++;
                }
            }
        }
    }
}

// Main Application
void app_main(void) {
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    display_init();

    uart_init();

    // Create MAVLink Receive Task
    xTaskCreate(mavlink_receive_task, "mavlink_receive_task", 8192, NULL, 10, NULL);

    // Main Loop
    while (1)
    {
        send_heartbeat();
        mavlink_calibrate_send_process();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
