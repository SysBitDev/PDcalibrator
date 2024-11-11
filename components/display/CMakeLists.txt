idf_component_register(SRCS "display.c"
                    INCLUDE_DIRS "include" "."
                    REQUIRES driver espressif__esp_lcd_touch espressif__esp_lcd_touch_gt911 lvgl)
