#include "display.h"
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

// Tag used for logging messages from this module
static const char *TAG = "display";

// Semaphore for synchronizing access to LVGL (LittlevGL) library
static SemaphoreHandle_t lvgl_mux = NULL;

// Handle for the touch controller
static esp_lcd_touch_handle_t touch_handle = NULL;

// Handle for the LCD panel
static esp_lcd_panel_handle_t panel_handle = NULL;

#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
// Semaphore to signal when a transaction is ready to avoid tearing effects
static SemaphoreHandle_t trans_ready_sem;
#endif

// Forward declarations of callback functions and tasks
static void lvgl_tick_inc_cb(void *arg);
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);
static void lvgl_task(void *arg);
static bool display_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data);

/**
 * @brief Initializes the display module.
 *
 * This function sets up the I2C interface, configures GPIOs, initializes the LCD panel,
 * touch controller, and the LVGL library. It also creates necessary FreeRTOS tasks and
 * synchronization primitives.
 */
void display_init(void)
{
    ESP_LOGI(TAG, "Initializing display");

    // Configure I2C parameters
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_I2C_MASTER_SDA_IO,
        .scl_io_num = CONFIG_I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = CONFIG_I2C_MASTER_FREQ_HZ,
    };

    // Initialize I2C driver
    ESP_ERROR_CHECK(i2c_param_config(CONFIG_I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(CONFIG_I2C_MASTER_NUM, conf.mode, 0, 0, 0));

    ESP_LOGI(TAG, "I2C initialized successfully");

    // Configure GPIO for touch controller reset
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << CONFIG_GPIO_TOUCH_RESET),
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Configure RGB LCD panel parameters
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16, // Number of data lines
        .psram_trans_align = 64, // Alignment for PSRAM transactions
#if CONFIG_EXAMPLE_DOUBLE_FB
        .num_fbs = 2, // Number of frame buffers for double buffering
#else
        .num_fbs = 1, // Single frame buffer
#endif
        .clk_src = LCD_CLK_SRC_DEFAULT, // Clock source
        .disp_gpio_num = CONFIG_PIN_NUM_DISP_EN, // Display enable pin
        .pclk_gpio_num = CONFIG_PIN_NUM_PCLK, // Pixel clock pin
        .vsync_gpio_num = CONFIG_PIN_NUM_VSYNC, // Vertical sync pin
        .hsync_gpio_num = CONFIG_PIN_NUM_HSYNC, // Horizontal sync pin
        .de_gpio_num = CONFIG_PIN_NUM_DE, // Data enable pin
        .data_gpio_nums = { // Data pins
            CONFIG_PIN_NUM_DATA0,
            CONFIG_PIN_NUM_DATA1,
            CONFIG_PIN_NUM_DATA2,
            CONFIG_PIN_NUM_DATA3,
            CONFIG_PIN_NUM_DATA4,
            CONFIG_PIN_NUM_DATA5,
            CONFIG_PIN_NUM_DATA6,
            CONFIG_PIN_NUM_DATA7,
            CONFIG_PIN_NUM_DATA8,
            CONFIG_PIN_NUM_DATA9,
            CONFIG_PIN_NUM_DATA10,
            CONFIG_PIN_NUM_DATA11,
            CONFIG_PIN_NUM_DATA12,
            CONFIG_PIN_NUM_DATA13,
            CONFIG_PIN_NUM_DATA14,
            CONFIG_PIN_NUM_DATA15,
        },
        .timings = { // Display timings
            .pclk_hz = CONFIG_LCD_PIXEL_CLOCK_HZ, // Pixel clock frequency
            .h_res = CONFIG_LCD_H_RES, // Horizontal resolution
            .v_res = CONFIG_LCD_V_RES, // Vertical resolution
            .hsync_back_porch = 8, // Horizontal sync back porch
            .hsync_front_porch = 8, // Horizontal sync front porch
            .hsync_pulse_width = 4, // Horizontal sync pulse width
            .vsync_back_porch = 16, // Vertical sync back porch
            .vsync_front_porch = 16, // Vertical sync front porch
            .vsync_pulse_width = 4, // Vertical sync pulse width
            .flags.pclk_active_neg = true, // Pixel clock active edge
        },
        .flags.fb_in_psram = true, // Frame buffer located in PSRAM
#if CONFIG_EXAMPLE_USE_BOUNCE_BUFFER
        .bounce_buffer_size_px = CONFIG_LCD_H_RES * 20, // Size of bounce buffer
#endif
    };

    ESP_LOGI(TAG, "Installing RGB LCD panel driver");
    // Create a new RGB LCD panel
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));

    ESP_LOGI(TAG, "Registering event callbacks");
    // Register event callbacks for the LCD panel
    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_vsync = display_on_vsync_event, // Callback for vertical sync
    };
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, NULL));

    ESP_LOGI(TAG, "Initializing RGB LCD panel");
    // Reset and initialize the LCD panel
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    ESP_LOGI(TAG, "Initializing touch controller GT911");
    // Configure and initialize the touch controller (GT911)
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)CONFIG_I2C_MASTER_NUM, &tp_io_config, &tp_io_handle));

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = CONFIG_LCD_V_RES, // Maximum X coordinate
        .y_max = CONFIG_LCD_H_RES, // Maximum Y coordinate
        .rst_gpio_num = -1, // Reset GPIO (not used)
        .int_gpio_num = -1, // Interrupt GPIO (not used)
        .flags = { // Touch controller flags
            .swap_xy = 0, // Do not swap X and Y coordinates
            .mirror_x = 0, // Do not mirror X axis
            .mirror_y = 0, // Do not mirror Y axis
        },
    };

    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &touch_handle));

    ESP_LOGI(TAG, "Initializing LVGL library");
    // Initialize the LVGL library
    lv_init();

    // Initialize display buffer for LVGL
    static lv_disp_draw_buf_t draw_buf;
    void *buf1 = NULL;
#if CONFIG_EXAMPLE_DOUBLE_FB
    void *buf2 = NULL;
    buf1 = heap_caps_malloc(CONFIG_LCD_H_RES * CONFIG_LCD_V_RES * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    buf2 = heap_caps_malloc(CONFIG_LCD_H_RES * CONFIG_LCD_V_RES * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf1 && buf2); // Ensure memory allocation was successful
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, CONFIG_LCD_H_RES * CONFIG_LCD_V_RES);
#else
    buf1 = heap_caps_malloc(CONFIG_LCD_H_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf1); // Ensure memory allocation was successful
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, CONFIG_LCD_H_RES * 100);
#endif

    // Initialize and register LVGL display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = CONFIG_LCD_H_RES; // Horizontal resolution
    disp_drv.ver_res = CONFIG_LCD_V_RES; // Vertical resolution
    disp_drv.flush_cb = lvgl_flush_cb; // Flush callback
    disp_drv.draw_buf = &draw_buf; // Display buffer
    disp_drv.user_data = panel_handle; // User data (LCD panel handle)
#if CONFIG_EXAMPLE_DOUBLE_FB
    disp_drv.full_refresh = true; // Enable full refresh for double buffering
#endif
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Installing LVGL tick timer");
    // Create and start a periodic timer for LVGL ticks
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_tick_inc_cb,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, CONFIG_LVGL_TICK_PERIOD_MS * 1000));

    // Initialize and register LVGL input device driver for touch
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER; // Pointer type (e.g., touch)
    indev_drv.disp = disp; // Associated display
    indev_drv.read_cb = lvgl_touch_cb; // Touch read callback
    indev_drv.user_data = touch_handle; // User data (touch controller handle)
    lv_indev_drv_register(&indev_drv);

    // Create a recursive mutex for LVGL to allow nested locking
    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    assert(lvgl_mux); // Ensure mutex was created successfully

#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
    // Create a binary semaphore to signal transaction readiness
    trans_ready_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(trans_ready_sem); // Initialize semaphore to available state
#endif

    ESP_LOGI(TAG, "Creating LVGL task");
    // Create a FreeRTOS task for handling LVGL operations
    xTaskCreate(lvgl_task, "lvgl_task", CONFIG_LVGL_TASK_STACK_SIZE, NULL, CONFIG_LVGL_TASK_PRIORITY, NULL);
}

/**
 * @brief LVGL task responsible for handling LVGL timers and rendering.
 *
 * This FreeRTOS task runs continuously, periodically calling `lv_timer_handler()`
 * to process LVGL tasks and update the display. It uses a recursive mutex to ensure
 * thread-safe access to LVGL functions.
 *
 * @param arg Unused parameter.
 */
static void lvgl_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    while (1) {
        if (display_lvgl_lock(-1)) { // Lock LVGL with indefinite timeout
            lv_timer_handler(); // Handle LVGL tasks
            display_lvgl_unlock(); // Unlock LVGL
        }
        vTaskDelay(pdMS_TO_TICKS(5)); // Delay to yield CPU
    }
}

/**
 * @brief Callback function to increment LVGL ticks.
 *
 * This function is called periodically by the LVGL tick timer to increment the
 * internal tick count of LVGL, which is used for animations and timing within the library.
 *
 * @param arg Unused parameter.
 */
static void lvgl_tick_inc_cb(void *arg)
{
    lv_tick_inc(CONFIG_LVGL_TICK_PERIOD_MS); // Increment LVGL tick count
}

/**
 * @brief Locks the LVGL library for thread-safe operations.
 *
 * This function attempts to take the LVGL mutex within a specified timeout.
 * If successful, it returns `true`, allowing the caller to perform LVGL operations.
 *
 * @param timeout_ms Timeout in milliseconds to wait for the mutex.
 *                   Use `-1` for an indefinite wait.
 *
 * @return
 *      - `true` if the mutex was successfully taken.
 *      - `false` if the mutex could not be taken within the timeout.
 */
bool display_lvgl_lock(int timeout_ms)
{
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_mux, timeout_ticks) == pdTRUE;
}

/**
 * @brief Unlocks the LVGL library after thread-safe operations.
 *
 * This function releases the LVGL mutex, allowing other tasks to access LVGL.
 */
void display_lvgl_unlock(void)
{
    xSemaphoreGiveRecursive(lvgl_mux); // Release the LVGL mutex
}

/**
 * @brief Flush callback function for LVGL to update the display.
 *
 * This function is called by LVGL when a portion of the screen needs to be updated.
 * It transfers the pixel data to the LCD panel within the specified area.
 *
 * @param drv Pointer to the LVGL display driver.
 * @param area Pointer to the area of the display to update.
 * @param color_map Pointer to the pixel data to be rendered.
 */
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
    // Wait for the previous transaction to be ready to avoid tearing
    xSemaphoreTake(trans_ready_sem, portMAX_DELAY);
#endif

    // Retrieve the LCD panel handle from the display driver user data
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;

    // Calculate the area to draw
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    // Draw the bitmap to the LCD panel
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);

    // Inform LVGL that flushing is done
    lv_disp_flush_ready(drv);
}

/**
 * @brief Touch input callback function for LVGL.
 *
 * This function is called by LVGL to read the current state of the touch input device.
 * It retrieves touch coordinates and updates the LVGL input data accordingly.
 *
 * @param drv Pointer to the LVGL input device driver.
 * @param data Pointer to the LVGL input data structure to be updated.
 */
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
        data->point.x = touchpad_x[0]; // Set X coordinate
        data->point.y = touchpad_y[0]; // Set Y coordinate
        data->state = LV_INDEV_STATE_PR; // Set state to pressed
    } else {
        data->state = LV_INDEV_STATE_REL; // Set state to released
    }
}

/**
 * @brief Event callback for vertical synchronization (VSYNC).
 *
 * This function is called during the VSYNC event of the LCD panel. It is used to
 * synchronize display updates and avoid tearing effects by signaling the semaphore.
 *
 * @param panel Handle to the LCD panel.
 * @param event_data Pointer to the event data structure.
 * @param user_data Pointer to user data (unused).
 *
 * @return
 *      - `true` if a higher priority task was woken by the semaphore release.
 *      - `false` otherwise.
 */
static bool display_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data)
{
#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
    BaseType_t high_task_wakeup = pdFALSE;
    // Release the semaphore from ISR context
    xSemaphoreGiveFromISR(trans_ready_sem, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
#else
    return false;
#endif
}
