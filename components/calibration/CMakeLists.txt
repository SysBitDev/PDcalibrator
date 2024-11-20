idf_component_register(
    SRCS 
        "src/_crc.c"
        "src/_debug.c"
        "src/_dsp.c"
        "src/_fifo.c"
        "src/_filter.c"
        "src/_misc.c"
        "src/colors.c"
        "src/fd_dsp.c"
        "src/modGUI.c"
        "src/modPaint.c"
        "src/modParser.c"
        "src/xprintf.c"
        "src/touch.c"
    INCLUDE_DIRS 
        "include"
    REQUIRES lvgl driver espressif__esp_lcd_touch_gt911 espressif__esp_lcd_touch
)

set_source_files_properties(
    src/_crc.c
    src/_debug.c
    src/_dsp.c
    src/_fifo.c
    src/_filter.c
    src/_misc.c
    src/colors.c
    src/fd_dsp.c
    src/modGUI.c
    src/modPaint.c
    src/modParser.c
    src/xprintf.c
    src/touch.c
    PROPERTIES COMPILE_FLAGS "-Wno-error"
)

