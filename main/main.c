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
#include "esp_system.h" // For esp_random()
#include <stdlib.h>     // For rand()

static const char *TAG = "display";

// Semaphore and handles
static SemaphoreHandle_t lvgl_mux = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

// Global variables for the arc and button
static lv_obj_t *arc = NULL;
static lv_obj_t *reset_btn = NULL;

// Define GPIOs for Backlight Control
#define LCD_LEDA_GPIO GPIO_NUM_2   // Example, replace with actual
#define LCD_LEDK_GPIO GPIO_NUM_3   // Example, replace with actual

// Define I2C Pins for GT911
#define TOUCH_GT911_SCL 20
#define TOUCH_GT911_SDA 19

// Forward declarations
static void lvgl_tick_inc_cb(void *arg);
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);
static void lvgl_task(void *arg);
static bool display_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data);
static void reset_button_event_handler(lv_event_t *e);
static void animate_arc(void);

// Function to configure backlight
void configure_backlight(void) {
    // Configure LCD_LEDA
    gpio_config_t backlight_leda_cfg = {
        .pin_bit_mask = (1ULL << LCD_LEDA_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&backlight_leda_cfg));
    gpio_set_level(LCD_LEDA_GPIO, 1); // Turn on LCD_LEDA

    // Configure LCD_LEDK
    gpio_config_t backlight_ledk_cfg = {
        .pin_bit_mask = (1ULL << LCD_LEDK_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&backlight_ledk_cfg));
    gpio_set_level(LCD_LEDK_GPIO, 1); // Turn on LCD_LEDK
}

// Function to create and initialize the arc with animation
void draw_arc(void) {
    if (arc != NULL) {
        lv_obj_del(arc); // Delete previous arc if exists
    }

    ESP_LOGI(TAG, "Creating arc");
    arc = lv_arc_create(lv_scr_act()); // Create new arc on active screen
    lv_obj_set_size(arc, 200, 200); // Set size as needed

    // Align to center
    lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);

    // Initialize arc angles to 0 for animation
    lv_arc_set_rotation(arc, 0); // Start rotation
    lv_arc_set_bg_angles(arc, 0, 360); // Full background circle
    lv_arc_set_angles(arc, 0, 0); // Start with 0 degrees

    // Set arc style
    lv_obj_set_style_arc_color(arc, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT); // Red color
    lv_obj_set_style_arc_width(arc, 2, LV_PART_MAIN | LV_STATE_DEFAULT); // Thin line
}

// Function to create the reset button
void create_reset_button(void) {
    ESP_LOGI(TAG, "Creating reset button");
    reset_btn = lv_btn_create(lv_scr_act()); // Create button on active screen
    lv_obj_set_size(reset_btn, 100, 50); // Set button size
    lv_obj_align(reset_btn, LV_ALIGN_BOTTOM_MID, 0, -20); // Align button at bottom center with offset

    // Add label to the button
    lv_obj_t *label = lv_label_create(reset_btn);
    lv_label_set_text(label, "Reset");
    lv_obj_center(label);

    // Add event handler for the button
    lv_obj_add_event_cb(reset_btn, reset_button_event_handler, LV_EVENT_CLICKED, NULL);
}

// Event handler for the reset button
static void reset_button_event_handler(lv_event_t *e) {
    ESP_LOGI(TAG, "Reset button pressed");
    draw_arc(); // Reset and redraw the arc
    animate_arc(); // Start the animation
}

// Function to animate the arc
static void animate_arc(void) {
    ESP_LOGI(TAG, "Starting arc animation");

    // Initialize LVGL animation
    static lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_arc_set_angles);
    lv_anim_set_values(&a, 0, 360); // Animate from 0 to 360 degrees
    lv_anim_set_time(&a, 2000); // 2 seconds
    lv_anim_set_playback_delay(&a, 0);
    lv_anim_set_ready_cb(&a, NULL);
    lv_anim_start(&a);

    ESP_LOGI(TAG, "Arc animation started");
}

// Function to initialize the display
void display_init(void) {
    ESP_LOGI(TAG, "Initializing display");

    // Configure backlight
    configure_backlight();

    // I2C Configuration for touch controller
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TOUCH_GT911_SDA, // GPIO 19
        .scl_io_num = TOUCH_GT911_SCL, // GPIO 20
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    ESP_LOGI(TAG, "Configuring I2C for touch controller...");
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0));
    ESP_LOGI(TAG, "I2C initialized successfully for touch controller");

    // RGB Panel Configuration
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16,
        .psram_trans_align = 64,
        .num_fbs = 1,  // Use one frame buffer
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num = LCD_LEDA_GPIO, // Use LCD_LEDA_GPIO for backlight
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

    // Initialize touch controller
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
        .rst_gpio_num = -1,  // Set to -1 since we handle reset manually
        .int_gpio_num = -1,  // Set to -1
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

    // Initialize LVGL library
    ESP_LOGI(TAG, "Initializing LVGL library");
    lv_init();

    // Allocate memory for LVGL display buffer
    static lv_disp_draw_buf_t draw_buf;
    void *buf1 = heap_caps_malloc(800 * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf1);
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, 800 * 100);

    // Initialize LVGL display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 800;
    disp_drv.ver_res = 480;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_drv_register(&disp_drv);

    // Initialize LVGL input device driver for touch
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_cb;
    indev_drv.user_data = touch_handle; // Pass touch_handle via user_data
    lv_indev_drv_register(&indev_drv);

    // Install LVGL tick timer
    ESP_LOGI(TAG, "Installing LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_tick_inc_cb,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, 10 * 1000)); // 10 ms

    // Create a recursive mutex for LVGL
    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    assert(lvgl_mux);

    // Create LVGL task
    ESP_LOGI(TAG, "Creating LVGL task");
    xTaskCreate(lvgl_task, "lvgl_task", 4096, NULL, 5, NULL);

    // Set black background
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Draw the initial arc
    draw_arc();

    // Start the arc animation
    animate_arc();

    // Create the reset button
    create_reset_button();
}

// VSYNC event callback
static bool display_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data) {
    // Additional logic on VSync if needed
    return false;
}

// Increment LVGL tick count for timing
static void lvgl_tick_inc_cb(void *arg) {
    lv_tick_inc(10); // 10 ms
}

// Flush display buffer to LCD
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

// Read touch input data
static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    // Read touch data from the touch controller
    esp_lcd_touch_read_data(drv->user_data);

    // Get touch coordinates
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

// LVGL task to process LVGL tasks and update display
static void lvgl_task(void *arg) {
    while (1) {
        if (xSemaphoreTakeRecursive(lvgl_mux, portMAX_DELAY) == pdTRUE) {
            lv_timer_handler();
            xSemaphoreGiveRecursive(lvgl_mux);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// Entry point for ESP-IDF application
void app_main(void) {
    // Set log level for 'display' tag to DEBUG for detailed logs
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    // Initialize display and LVGL
    display_init();
}
