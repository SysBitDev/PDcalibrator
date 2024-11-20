// components/calibration/include/touch.h

#ifndef TOUCH_H
#define TOUCH_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

// Виберіть потрібний контролер торкання, закоментувавши або розкоментувавши відповідні рядки

/* Для FT6X36 */
// #define TOUCH_FT6X36
// #define TOUCH_FT6X36_SCL GPIO_NUM_19
// #define TOUCH_FT6X36_SDA GPIO_NUM_18
// #define TOUCH_FT6X36_INT GPIO_NUM_39
// #define TOUCH_SWAP_XY
// #define TOUCH_MAP_X1 480
// #define TOUCH_MAP_X2 0
// #define TOUCH_MAP_Y1 0
// #define TOUCH_MAP_Y2 320

/* Для GT911 */
#define TOUCH_GT911
#define TOUCH_GT911_SCL GPIO_NUM_20
#define TOUCH_GT911_SDA GPIO_NUM_19
#define TOUCH_GT911_INT -1
#define TOUCH_GT911_RST -1
#define TOUCH_GT911_ROTATION 0
#define TOUCH_MAP_X1 800
#define TOUCH_MAP_X2 0
#define TOUCH_MAP_Y1 480
#define TOUCH_MAP_Y2 0

/* Для XPT2046 */
// #define TOUCH_XPT2046
// #define TOUCH_XPT2046_SCK GPIO_NUM_12
// #define TOUCH_XPT2046_MISO GPIO_NUM_13
// #define TOUCH_XPT2046_MOSI GPIO_NUM_11
// #define TOUCH_XPT2046_CS GPIO_NUM_38
// #define TOUCH_XPT2046_INT GPIO_NUM_18
// #define TOUCH_XPT2046_ROTATION 0
// #define TOUCH_MAP_X1 4000
// #define TOUCH_MAP_X2 100
// #define TOUCH_MAP_Y1 100
// #define TOUCH_MAP_Y2 4000

// Оголошення зовнішніх змінних для координат торкання
extern int touch_last_x;
extern int touch_last_y;

// Прототипи функцій
esp_err_t touch_init(void);
bool touch_has_signal(void);
bool touch_touched(void);
bool touch_released(void);
void touch_read(void);

#endif // TOUCH_H
