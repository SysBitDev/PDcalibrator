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

#include <MAVLink.h>
#include "conf.h"
#include "mavlink_msg_fd.h"
#include "mavlink_msg_fd_clb.h"

#include <lvgl.h>
#include <TFT_eSPI.h> // Використовується драйвер TFT_eSPI для LVGL
#include "touch.h"

#include <SPI.h>
#include <SD.h>
#include <FS.h>

#define SD_MOSI 11
#define SD_MISO 13
#define SD_SCK 12
#define SD_CS 10

#include "Audio.h"
Audio audio;

#define I2S_DOUT      17
#define I2S_BCLK      42
#define I2S_LRC       18

// Ініціалізація TFT дисплея
TFT_eSPI tft = TFT_eSPI(); // Створення об'єкта для TFT дисплея

// Буфер для LVGL дисплея
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[LV_HOR_RES_MAX * 10];
static lv_color_t buf2[LV_HOR_RES_MAX * 10];
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

// Об'єкт LVGL Canvas для малювання
lv_obj_t *canvas;

// Прототипи функцій
void my_flush_cb(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p);
bool my_touch_read(lv_indev_drv_t * drv, lv_indev_data_t * data);

// Функція обробки оновлення дисплея
void my_flush_cb(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p) {
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, area->x2 - area->x1 +1, area->y2 - area->y1 +1);
    tft.pushColors((uint16_t *)&color_p->full, (area->x2 - area->x1 +1) * (area->y2 - area->y1 +1), true);
    tft.endWrite();
    lv_disp_flush_ready(disp);
}

// Функція обробки сенсорних даних
bool my_touch_read(lv_indev_drv_t * drv, lv_indev_data_t * data) {
    if(touch_touched()) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touch_last_x;
        data->point.y = touch_last_y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
    return false; // Не більше даних для читання
}

// Функція ініціалізації LVGL
void setup_lvgl() {
    lv_init();

    tft.begin();
    tft.setRotation(1); // Встановіть потрібну орієнтацію

    // Ініціалізація буфера для LVGL
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LV_HOR_RES_MAX * 10);

    // Ініціалізація дисплейного драйвера LVGL
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 800;
    disp_drv.ver_res = 480;
    disp_drv.flush_cb = my_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.rounder_cb = NULL;
    disp_drv.vsync_cb = NULL;
    lv_disp_drv_register(&disp_drv);

    // Ініціалізація драйвера для сенсорного екрана
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touch_read;
    lv_indev_drv_register(&indev_drv);

    // Створення Canvas для малювання
    canvas = lv_canvas_create(lv_scr_act(), NULL);
    lv_canvas_set_buffer(canvas, buf1, LV_HOR_RES_MAX, LV_VER_RES_MAX, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_pos(canvas, 0, 0);
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
}

// Глобальні змінні ======================================================
CHAR str[256];

#include "_filter.h"
filter_window_FLOAT_t filt_0;
//BOOL msg_ready = FALSE;

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

//BOOL _sound_play = 0;
void sound_play( const char *str )
{
    audio.connecttoFS( SD, str );
    //audio.connecttospeech( "TEST", "en");
    do
    {
        //delay(1);
        audio.loop();
    }
    while( audio.isRunning() );
}

int fd_dsp_touch_has_signal( void )
{
    return touch_has_signal();
}

int fd_dsp_touch_touched( void )
{
    return touch_touched();
}

int fd_dsp_touch_released( void )
{
    return touch_released();
}

INT fd_dsp_button_test( button_t *b )
{
    INT ret = 0;

    if( touch_last_x >= b->x && touch_last_x <= (b->x + b->w) )
    {
        if( touch_last_y >= b->y && touch_last_y <= (b->y + b->h) )
        {
            ret = 1;
        }
    }

    if( 1 == ret ) //send sound
    {
        sound_play( "/0001_blip1.mp3" ); //sound_play( "/0000_blip.mp3" );
        //delay( 1300 );
    }

    return ret;
}

static FLOAT calibrate_buf[ FD_CLB_DOTS * 2 ];
INT mavlink_send_state = 0;

// Функція для скидання калібрування
void fd_dsp_calibrate_reset( void )
{
    if( CMD_NOP == mavlink_send_state )
        mavlink_send_state = CMD_FLASH_ERASE;
}

// Функція для запису калібрування
void fd_dsp_calibrate_write( void )
{
    if( CMD_NOP == mavlink_send_state )
        mavlink_send_state = CMD_FLASH_WRITE;
}

// Функція для обробки процесу відправки калібрування через MAVLink
void fd_dsp_calibrate_send_process( void )
{
    mavlink_message_t msg;
    mavlink_fd_clb_t mav_fd_clb;
    static INT iter = 0;
    static INT delay_cnt = 0;
    CHAR str_local[32];

    switch( mavlink_send_state )
    {
    case CMD_NOP: {
        iter = 0;
    } break;

    case CMD_FLASH_ERASE: {
        xsprintf( str_local, "Sending erase..." );
        // Створення та налаштування лейбла для відображення повідомлення
        lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
        lv_label_set_text(label, str_local);
        lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
        lv_obj_set_pos(label, 310, 20);

        if( 0 == delay_cnt )
        {
            mav_fd_clb.cmd = CMD_FLASH_ERASE;
            mav_fd_clb.data_num = 0;
            mav_fd_clb.idnum = fsd.idnum;

            mavlink_msg_fd_clb_encode( SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb );
            INT size = mavlink_msg_to_send_buffer( parser_tx_buf, &msg );

            Serial.write( parser_tx_buf, size );
        }
        delay_cnt++;
        if( 30 == delay_cnt )
        {
            delay_cnt = 0;
            mavlink_send_state = CMD_NOP;
        }
    } break;

    case CMD_FLASH_WRITE: {
        xsprintf( str_local, "Sending %u...", iter );
        // Створення та налаштування лейбла для відображення повідомлення
        lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
        lv_label_set_text(label, str_local);
        lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
        lv_obj_set_pos(label, 310, 20);

        delay_cnt++;
        if( 5 == delay_cnt )
        {
            delay_cnt = 0;
            if( iter < 360 / 8 )
            {
                mav_fd_clb.cmd = CMD_FLASH_WRITE;
                mav_fd_clb.data_num = iter;
                mav_fd_clb.idnum = fsd.idnum;

                for( INT j = 0; j < 8; j++ )
                {
                    mav_fd_clb.clb_val[ j ] = calibrate_buf[ iter*8 + j ];
                }

                mavlink_msg_fd_clb_encode( SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb );
                INT size = mavlink_msg_to_send_buffer( parser_tx_buf, &msg );

                Serial.write( parser_tx_buf, size );
            }
            else
            {
                mavlink_send_state = CMD_FLASH_FINALIZE;
            }
            iter++;
        }
    } break;

    case CMD_FLASH_FINALIZE: {
        xsprintf( str_local, "Sending finished..." );
        // Створення та налаштування лейбла для відображення повідомлення
        lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
        lv_label_set_text(label, str_local);
        lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
        lv_obj_set_pos(label, 310, 20);

        mav_fd_clb.cmd = CMD_FLASH_FINALIZE;
        mav_fd_clb.data_num = 0;
        mav_fd_clb.idnum = fsd.idnum;

        mavlink_msg_fd_clb_encode( SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb );
        INT size = mavlink_msg_to_send_buffer( parser_tx_buf, &msg );

        Serial.write( parser_tx_buf, size );

        mavlink_send_state = CMD_NOP;
    } break;

    default: break;
    }
}

// Функція для малювання на екрані
void drawfunc(void)
{
    static U32 sec = 0;
    static U32 psec = 0;
    sec = millis() / ( 1000 / 20 ); // Оновлення 20 разів на секунду
    if( psec != sec )
    {
        psec = sec;

        // Очищення Canvas
        lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

        // Малювання DSP даних
        fd_dsp_draw_all( &fd_data, &fsd, mode, filter_mode, mul );
        fd_dsp_buttons_processor( &mode, &filter_mode, &mul );

        // Відображення FPS та MAVPS
        xsprintf( str, "fps:%d m:%u", fps, mavps );
        lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
        lv_label_set_text(label, str);
        lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_pos(label, 620, 2);

        frame_count++;
    }
}

//==============================================================================
void setup(void)
{
    // Ініціалізація серійного порту
    Serial.setRxBufferSize(4096);
    Serial.begin(921600);

    // Ініціалізація сенсорного екрану
    touch_init();

    // Ініціалізація LVGL
    setup_lvgl();

    // Ініціалізація SD-карти
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
    SPI.setFrequency(16000000);
    SD.begin(SD_CS);

    // Ініціалізація аудіо
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(9); // 0...21

    // Ініціалізація графічного інтерфейсу
    hal_screen_startup_out();

    // Заповнення екрану чорним кольором
    hal_paint_screen_fill(COLOR_BLACK);

    // Відтворення стартового звуку
    sound_play("/0018_battery_pickup.mp3");

    // Ініціалізація DSP
    fd_dsp_draw_init(&fd_data);

#if UART_PROTOCOL_MAVLINK
    // Ініціалізація MAVLink, якщо потрібно
#else
    // Ініціалізація парсерів
    modParser_init(&parser_rx, PIK_ADR_DEFAULT, PARSER_MAX_DATA_SIZE);
    modParser_init(&parser_tx, PIK_ADR_DEFAULT, PARSER_MAX_DATA_SIZE);
#endif

    Serial.printf("\r\nINIT ALL OK!\r\n");
}

void sendMAVLink( void )
{
    fd_dsp_calibrate_send_process();

    static U32 lastSent = 0;
    if( millis() - lastSent < 1000 ) return; // Відправка кожну секунду

    lastSent = millis();

    // Генерація HEARTBEAT повідомлення
    mavlink_message_t msg;
    U8 buf[ MAVLINK_MAX_PACKET_LEN ];

    mavlink_msg_heartbeat_pack( 1, MAV_COMP_ID_AUTOPILOT1, &msg, MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, MAV_MODE_FLAG_MANUAL_INPUT_ENABLED, 0, MAV_STATE_STANDBY );
    U16 len = mavlink_msg_to_send_buffer( buf, &msg );

    Serial.write( buf, len );
}

void fd_dsp_MAVLink_recive( void ) // Парсинг MAVLink повідомлень
{
    static mavlink_message_t msg;
    static mavlink_status_t status;

    U32 cnt_max = 4000;
    while( Serial.available() > 0 && cnt_max-- )
    {
        char tmp_char = Serial.read(); // Отримання вхідного байта
        serial_chars_cnt++;

#if UART_PROTOCOL_MAVLINK
        if( mavlink_parse_char( MAVLINK_COMM_0, tmp_char, &msg, &status ))
        {
            // Обробка MAVLink повідомлень
            if( MAVLINK_MSG_ID_FD == msg.msgid )
            {
                mavlink_fd_t mav_fd = {0};
                mavlink_msg_fd_decode( &msg, &mav_fd );

                fsd.version = mav_fd.version;
                // fsd.calibrated = mav_fd.calibrated;
                fsd.error_code = mav_fd.error_code;
                // fsd.ch_num = mav_fd.ch_num;
                // fsd.branches_num = mav_fd.branches_num;
                // fsd.vector_mode = mav_fd.vector_mode;
                fsd.inc_cnt = mav_fd.inc_cnt;

                fsd.idnum = mav_fd.idnum;

                // RSSI після 1-x каскаду
                fd_data.v[0] = mav_fd.b1ch3; // для ch1
                fd_data.v[1] = mav_fd.b1ch1; // для ch2
                fd_data.v[2] = mav_fd.b1ch2; // для ch3
                fd_data.v[3] = mav_fd.b1ch4; // для ch4

                // RSSI після 2-x каскаду
                fd_data.v[4] = mav_fd.b2ch3; // для ch1
                fd_data.v[5] = mav_fd.b2ch1; // для ch2
                fd_data.v[6] = mav_fd.b2ch2; // для ch3
                fd_data.v[7] = mav_fd.b2ch4; // для ch4

                // SNR рівень після 2-x каскаду
                fd_data.snr[2] = mav_fd.snr_b1ch1; // для ch1
                fd_data.snr[3] = mav_fd.snr_b1ch2; // для ch2
                fd_data.snr[4] = mav_fd.snr_b1ch3; // для ch3
                fd_data.snr[5] = mav_fd.snr_b1ch4; // для ch4

                // SNR рівень після 2-x каскаду
                fd_data.snr[2] = mav_fd.snr_b2ch1; // для ch1
                fd_data.snr[3] = mav_fd.snr_b1ch2; // для ch2
                fd_data.snr[4] = mav_fd.snr_b2ch3; // для ch3
                fd_data.snr[5] = mav_fd.snr_b2ch4; // для ch4

                fd_data.fx[0] = mav_fd.x[0];
                fd_data.fx[1] = mav_fd.x[1];
                fd_data.fy[0] = mav_fd.y[0];
                fd_data.fy[1] = mav_fd.y[1];

                fd_dsp_vector_get( &fd_data, AREA_SIZE * mul );
                fd_data.fx[0] = fd_data.x[0];
                fd_data.fx[1] = fd_data.x[1];
                fd_data.fy[0] = fd_data.y[0];
                fd_data.fy[1] = fd_data.y[1];

                fd_dsp_add_new_vector( &fd_data );
                fd_dsp_calibrate_process( &fd_data, &calibrate_buf[0] );

                mavlink_msg_cnt++;
            }
        }
#else
        INT size;
        if( RET_OK == modParser_decode( &parser_rx, tmp_char, parser_rx_buf, &size ))
        {
            if( 0 == fd_dsp_rx_packet( (U8*)parser_rx_buf, &fsd, &fd_data ))
            {
                fd_dsp_add_new_vector( &fd_data, mul );
                mavlink_msg_cnt++;
            }
        }
#endif
    }
}

static void mainfunc(void)
{
    static U32 sec = 0;
    static U32 psec = 0;
    sec = millis() / 1000;
    if( psec != sec )
    {
        psec = sec;
        fps = frame_count;
        frame_count = 0;
        mavps = mavlink_msg_cnt;
        mavlink_msg_cnt = 0;
    }
}

void loop(void)
{
    // Оновлення LVGL
    lv_task_handler();
    delay(5); // Затримка для стабільності

    // Основні функції
    mainfunc();
    drawfunc();
    sendMAVLink();
    fd_dsp_MAVLink_recive();
}

//==============================================================================
extern "C" {

// Адаптовані функції для малювання з використанням LVGL

COORD hal_paint_screen_width( void )
{
  return 800; // LV_HOR_RES_MAX
}

COORD hal_paint_screen_height( void )
{
  return 480; // LV_VER_RES_MAX
}

RET hal_paint_init( INT mode )
{
    // У LVGL зазвичай не потрібні окремі ініціалізації для малювання,
    // але можна створити необхідні об'єкти тут, якщо потрібно.
    return RET_OK;
}

void hal_paint_pixel_set( COORD x, COORD y, COLOR color )
{
    // Встановлення пікселя на Canvas
    lv_color_t lv_color = lv_color_make((color >> 11) & 0x1F, (color >> 5) & 0x3F, color & 0x1F);
    lv_canvas_set_px(canvas, x, y, lv_color);
}

COLOR hal_paint_pixel_get( COORD x, COORD y )
{
    // Отримання пікселя з Canvas
    if( ( x >= hal_paint_screen_width() ) ||
        ( y >= hal_paint_screen_height() ) ||
            ( x < 0 ) || ( y < 0 ) )
    {
        return COLOR_BLACK;
    }
    else
    {
        lv_color_t lv_color = lv_canvas_get_px(canvas, x, y);
        return ( (lv_color.ch.red << 11) | (lv_color.ch.green << 5) | (lv_color.ch.blue) );
    }
}

void hal_paint_block( COORD x, COORD y, COORD w, COORD h, COLOR *color )
{
    for( COORD j = 0; j < h; j++)
    {
        for( COORD i = 0; i < w; i++)
        {
            hal_paint_pixel_set( i + x, j + y, *color++ );
        }
    }
}

void hal_paint_block_color( COORD x, COORD y, COORD w, COORD h, COLOR color )
{
    // Створення та заповнення прямокутника на Canvas
    lv_color_t lv_color = lv_color_make((color >> 11) & 0x1F, (color >> 5) & 0x3F, color & 0x1F);
    for( COORD j = 0; j < h; j++ )
    {
        for( COORD i = 0; i < w; i++ )
        {
            hal_paint_pixel_set( x + i, y + j, lv_color.full );
        }
    }
}

void hal_paint_block_transparent( COORD x, COORD y, COORD w, COORD h, COLOR *color, COLOR transparent )
{
    for( COORD j = 0; j < h; j++ )
    {
        for( COORD i = 0; i < w; i++ )
        {
            if( *color != transparent )
            {
                hal_paint_pixel_set( i + x, j + y, *color );
            }
            color++;
        }
    }
}

void hal_paint_screen_fill( COLOR color )
{
    // Заповнення всього Canvas певним кольором
    lv_color_t lv_color = lv_color_make((color >> 11) & 0x1F, (color >> 5) & 0x3F, color & 0x1F);
    lv_canvas_fill_bg(canvas, lv_color, LV_OPA_COVER);
}

RET hal_paint_screen_update( void )
{
    // Оновлення Canvas на дисплеї
    lv_obj_invalidate(canvas); // Позначаємо Canvas як потребуючий оновлення
    return RET_OK;
}

} // extern "C"
