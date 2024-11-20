// components/calibration/src/touch.c

#include "touch.h"

int touch_last_x = 0;
int touch_last_y = 0;

static const char *TAG = "Touch";

#if defined(TOUCH_FT6X36)
#include "esp_lcd_touch_ft5x06.h" // Використовується драйвер FT5x06 для сумісності з FT6X36

static esp_lcd_touch_handle_t touch_handle = NULL;
static bool touch_touched_flag = false;
static bool touch_released_flag = false;

#elif defined(TOUCH_GT911)
#include "esp_lcd_touch_gt911.h"

static esp_lcd_touch_handle_t touch_handle = NULL;

#elif defined(TOUCH_XPT2046)
#include "esp_lcd_touch_xpt2046.h"
#include "driver/spi_master.h"

static esp_lcd_touch_handle_t touch_handle = NULL;

#endif

// Допоміжна функція для мапінгу значень
static inline int32_t map_value(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min + 1) + out_min;
}

esp_err_t touch_init(void)
{
#if defined(TOUCH_GT911)
    // Ініціалізація I2C panel IO для GT911
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = {
        .dev_addr = 0x14, // Адреса I2C GT911 (змініть за потребою)
        .control_phase_bytes = 1, // Змінено з 0
        .dc_bit_offset = 0,
        .lcd_cmd_bits = 16,
        .lcd_param_bits = 8,
        .flags = {
            .disable_control_phase = 0, // Змінено з 1
        },
    };

    ESP_LOGI(TAG, "Initializing GT911 touch controller");

    esp_err_t ret = esp_lcd_new_panel_io_i2c(I2C_NUM_0, &tp_io_config, &tp_io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C panel IO for GT911: %s", esp_err_to_name(ret));
        return ret;
    }

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = TOUCH_MAP_X1,
        .y_max = TOUCH_MAP_Y1,
        .rst_gpio_num = TOUCH_GT911_RST,
        .int_gpio_num = TOUCH_GT911_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0, // Встановіть 1, якщо потрібно обміняти осі X та Y
            .mirror_x = 0, // Встановіть 1 для дзеркального відображення по осі X
            .mirror_y = 0, // Встановіть 1 для дзеркального відображення по осі Y
        },
    };

    ret = esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &touch_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize GT911 touch controller: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "GT911 touch controller initialized successfully");
    return ESP_OK;

#elif defined(TOUCH_FT6X36)
    // Реалізуйте ініціалізацію FT6X36 за допомогою драйверів ESP-IDF
    return ESP_OK;

#elif defined(TOUCH_XPT2046)
    // Реалізуйте ініціалізацію XPT2046 за допомогою драйверів ESP-IDF
    return ESP_OK;

#else
    ESP_LOGW(TAG, "No touch controller defined");
    return ESP_ERR_INVALID_ARG;
#endif
}

bool touch_has_signal(void)
{
#if defined(TOUCH_GT911)
    // Для GT911 завжди повертає true, оскільки ми опитуємо дані торкання
    return true;

#elif defined(TOUCH_FT6X36)
    // Реалізуйте для FT6X36 за потребою
    return touch_touched_flag || touch_released_flag;

#elif defined(TOUCH_XPT2046)
    // Реалізуйте для XPT2046 за потребою
    return true;

#else
    return false;
#endif
}

bool touch_touched(void)
{
#if defined(TOUCH_GT911)
    if (!touch_handle) {
        ESP_LOGW(TAG, "Touch handle not initialized");
        return false;
    }

    esp_lcd_touch_read_data(touch_handle);
    uint16_t touchpad_x[1];
    uint16_t touchpad_y[1];
    uint8_t touchpad_cnt = 0;

    bool touched = esp_lcd_touch_get_coordinates(
        touch_handle, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

    if (touched && touchpad_cnt > 0)
    {
        // За бажанням, обміняти X та Y
#if defined(TOUCH_SWAP_XY)
        uint16_t temp = touchpad_x[0];
        touchpad_x[0] = touchpad_y[0];
        touchpad_y[0] = temp;
#endif

        // Мапінг координат торкання
        touch_last_x = map_value(touchpad_x[0], 0, TOUCH_MAP_X1 - 1, TOUCH_MAP_X2, TOUCH_MAP_X1 - 1);
        touch_last_y = map_value(touchpad_y[0], 0, TOUCH_MAP_Y1 - 1, TOUCH_MAP_Y2, TOUCH_MAP_Y1 - 1);

        return true;
    }
    else
    {
        return false;
    }

#elif defined(TOUCH_FT6X36)
    // Реалізуйте touch_touched для FT6X36
    if (touch_touched_flag)
    {
        touch_touched_flag = false;
        return true;
    }
    else
    {
        return false;
    }

#elif defined(TOUCH_XPT2046)
    // Реалізуйте touch_touched для XPT2046
    return false;

#else
    return false;
#endif
}

bool touch_released(void)
{
#if defined(TOUCH_GT911)
    if (!touch_handle) {
        ESP_LOGW(TAG, "Touch handle not initialized");
        return false;
    }

    esp_lcd_touch_read_data(touch_handle);
    uint16_t touchpad_x[1];
    uint16_t touchpad_y[1];
    uint8_t touchpad_cnt = 0;

    bool touched = esp_lcd_touch_get_coordinates(
        touch_handle, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

    return !touched;

#elif defined(TOUCH_FT6X36)
    // Реалізуйте touch_released для FT6X36
    if (touch_released_flag)
    {
        touch_released_flag = false;
        return true;
    }
    else
    {
        return false;
    }

#elif defined(TOUCH_XPT2046)
    // Реалізуйте touch_released для XPT2046
    return false;

#else
    return false;
#endif
}

void touch_read(void)
{
    // Опційна функція для оновлення даних торкання, якщо необхідно
#if defined(TOUCH_GT911)
    // Дані опитуються у функціях touch_touched() та touch_released()
#elif defined(TOUCH_FT6X36)
    // Реалізуйте touch_read для FT6X36
#elif defined(TOUCH_XPT2046)
    // Реалізуйте touch_read для XPT2046
#endif
}
