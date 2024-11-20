
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

#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
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


// Define a class named LGFX, inheriting from the LGFX_Device class.
class LGFX : public lgfx::LGFX_Device {
public:
  // Instances for the RGB bus and panel.
  lgfx::Bus_RGB     _bus_instance;
  lgfx::Panel_RGB   _panel_instance;
  lgfx::Light_PWM   _light_instance;

  // Constructor for the LGFX class.
  LGFX(void) {

    // Configure the panel.
    {
      auto cfg = _panel_instance.config();
      cfg.memory_width  = 800;
      cfg.memory_height = 480;
      cfg.panel_width   = 800;
      cfg.panel_height  = 480;
      cfg.offset_x      = 0;
      cfg.offset_y      = 0;

      // Apply configuration to the panel instance.
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _panel_instance.config_detail();

      cfg.use_psram = 1;

      _panel_instance.config_detail(cfg);
    }


    // Configure the RGB bus.
    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;

      // Configure data pins.
      cfg.pin_d0  = GPIO_NUM_15; // B0
      cfg.pin_d1  = GPIO_NUM_7;  // B1
      cfg.pin_d2  = GPIO_NUM_6;  // B2
      cfg.pin_d3  = GPIO_NUM_5;  // B3
      cfg.pin_d4  = GPIO_NUM_4;  // B4
      
      cfg.pin_d5  = GPIO_NUM_9;  // G0
      cfg.pin_d6  = GPIO_NUM_46; // G1
      cfg.pin_d7  = GPIO_NUM_3;  // G2
      cfg.pin_d8  = GPIO_NUM_8;  // G3
      cfg.pin_d9  = GPIO_NUM_16; // G4
      cfg.pin_d10 = GPIO_NUM_1;  // G5
      
      cfg.pin_d11 = GPIO_NUM_14; // R0
      cfg.pin_d12 = GPIO_NUM_21; // R1
      cfg.pin_d13 = GPIO_NUM_47; // R2
      cfg.pin_d14 = GPIO_NUM_48; // R3
      cfg.pin_d15 = GPIO_NUM_45; // R4

      // Configure sync and clock pins.
      cfg.pin_henable = GPIO_NUM_41;
      cfg.pin_vsync   = GPIO_NUM_40;
      cfg.pin_hsync   = GPIO_NUM_39;
      cfg.pin_pclk    = GPIO_NUM_0;
      cfg.freq_write  = 14000000;

      // Configure timing parameters for horizontal and vertical sync.
      cfg.hsync_polarity    = 0;
      cfg.hsync_front_porch = 40;
      cfg.hsync_pulse_width = 48;
      cfg.hsync_back_porch  = 40;
      
      cfg.vsync_polarity    = 0;
      cfg.vsync_front_porch = 1;
      cfg.vsync_pulse_width = 31;
      cfg.vsync_back_porch  = 13;

      // Configure polarity for clock and data transmission.
      cfg.pclk_active_neg   = 1;
      cfg.de_idle_high      = 0;
      cfg.pclk_idle_high    = 0;

      // Apply configuration to the RGB bus instance.
      _bus_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = GPIO_NUM_2;
      _light_instance.config(cfg);
    }
    _panel_instance.light(&_light_instance);

    // Set the RGB bus and panel instances.
    _panel_instance.setBus(&_bus_instance);
    setPanel(&_panel_instance);
  }
};

LGFX lcd;
static LGFX_Sprite _sprites[2];

static std::uint32_t fps = 0, frame_count = 0;

// static std::uint32_t _width;
// static std::uint32_t _height;
U32 err = 0;
//COLOR color_current;
static int flip = 0;


static void diffDraw(LGFX_Sprite* sp0, LGFX_Sprite* sp1)
{
  union
  {
    std::uint32_t* s32;
    std::uint8_t* s;
  };
  union
  {
    std::uint32_t* p32;
    std::uint8_t* p;
  };
  s32 = (std::uint32_t*)sp0->getBuffer();
  p32 = (std::uint32_t*)sp1->getBuffer();

  auto width  = sp0->width();
  auto height = sp0->height();

  auto w32 = (width+3) >> 2;
  std::int32_t y = 0;
  do
  {
    std::int32_t x32 = 0;
    do
    {
      while (s32[x32] == p32[x32] && ++x32 < w32);
      if (x32 == w32) break;

      std::int32_t xs = x32 << 2;
      while (s[xs] == p[xs]) ++xs;

      while (++x32 < w32 && s32[x32] != p32[x32]);

      std::int32_t xe = (x32 << 2) - 1;
      if (xe >= width) xe = width - 1;
      while (s[xe] == p[xe]) --xe;

      lcd.pushImage(xs, y, xe - xs + 1, 1, &s[xs]);
    } while (x32 < w32);
    s32 += w32;
    p32 += w32;
  } while (++y < height);
  lcd.display();
}


// variables ======================================================
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
void fd_dsp_calibrate_reset( void )
{
    if( CMD_NOP == mavlink_send_state )
        mavlink_send_state = CMD_FLASH_ERASE;
}

void fd_dsp_calibrate_write( void )
{
    if( CMD_NOP == mavlink_send_state )
        mavlink_send_state = CMD_FLASH_WRITE;
}

void fd_dsp_calibrate_send_process( void )
{
    mavlink_message_t msg;
    mavlink_fd_clb_t mav_fd_clb;
    static INT iter = 0;
    static INT delay = 0;
    CHAR str[32];

    switch( mavlink_send_state )
    {
    case CMD_NOP: {
        iter = 0;
    } break;

    case CMD_FLASH_ERASE: {
        xsprintf( str, "Sending erase..." );
        paint_color_set( COLOR_ORANGE );
        paint_text_xy( 310, 20, str );

        if( 0 == delay )
        {
            mav_fd_clb.cmd = CMD_FLASH_ERASE;
            mav_fd_clb.data_num = 0;
            mav_fd_clb.idnum = fsd.idnum;

            mavlink_msg_fd_clb_encode( SYSTEM_ID, COMPONENT_ID, &msg, &mav_fd_clb );
            INT size = mavlink_msg_to_send_buffer( parser_tx_buf, &msg );

            Serial.write( parser_tx_buf, size );
        }
        delay++;
        if( 30 == delay )
        {
            delay = 0;
            mavlink_send_state = CMD_NOP;
        }
    } break;

    case CMD_FLASH_WRITE: {
        xsprintf( str, "Sending %u...", iter );
        paint_color_set( COLOR_ORANGE );
        paint_text_xy( 310, 20, str );

        delay++;
        if( 5 == delay )
        {
            delay = 0;
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
        xsprintf( str, "Sending finished..." );
        paint_color_set( COLOR_ORANGE );
        paint_text_xy( 310, 20, str );

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

void drawfunc(void)
{
    //static INT clear_cnt = 0;
    static U32 sec = 0;
    static U32 psec = 0;
    sec = lgfx::millis() / ( 1000 / 20 );
    if( psec != sec )
    {
        psec = sec;

        // if( ++clear_cnt > 100 )
        // {
            // COORD center_x = hal_paint_screen_width()/2;
    	      // COORD center_y = hal_paint_screen_height()/2 + 11;
            // paint_color( COLOR_BLACK );
            // paint_rectangle( center_x - AREA_SIZE, center_y - AREA_SIZE + 11, AREA_SIZE * 2, AREA_SIZE * 2, TRUE );
        //     //lcd.clear();
        //     clear_cnt = 0;
        // }
        paint_screen_clear();

        // paint_color( COLOR_BLUE );
        // lcd.fillCircle( touch_last_x, touch_last_y, 12 );

        fd_dsp_draw_all( &fd_data, &fsd, mode, filter_mode, mul );
        fd_dsp_buttons_processor( &mode, &filter_mode, &mul );

        xsprintf( str, "fps:%d m:%u", fps, mavps );
        paint_color_set( COLOR_WHITE );
        paint_text_xy( 620, 2, str );

        // lcd.setCursor( 380, 32 );
        // lcd.setTextColor( TFT_WHITE );
        // lcd.printf( "test"  );

        //lcd.display();
        hal_paint_screen_update();
        //diffDraw( &_sprites[flip], &_sprites[!flip] );

        frame_count++;
    }
}


//==============================================================================
void setup(void)
{
    // serial
    Serial.setRxBufferSize( 4096 );
    Serial.begin( 57600 );

    // touch
    touch_init();

    // lcd
    lcd.begin();
    lcd.startWrite();
    lcd.setColorDepth( 16 ) ;
    lcd.setTextSize( 1 );
    if (lcd.width() < lcd.height()) lcd.setRotation(lcd.getRotation() ^ 1);

    auto lcd_width = lcd.width();
    auto lcd_height = lcd.height();

    for( U32 i = 0; i < 2; ++i )
    {
        _sprites[i].setTextSize(2);
        _sprites[i].setColorDepth( 16 );
    }

    bool fail = false;
    for( U32 i = 0; !fail && i < 2; ++i )
    {
        fail = !_sprites[i].createSprite( lcd_width, lcd_height );
    }

    if( fail )
    {
        Serial.printf("fail-1\r\n" );
        fail = false;
        for( U32 i = 0; !fail && i < 2; ++i)
        {
            _sprites[i].setPsram( true );
            fail = !_sprites[i].createSprite( lcd_width, lcd_height );
        }

        if (fail)
        {
            // Serial.printf("fail-2\r\n" );

            // fail = false;
            // if (lcd_width > 320) lcd_width = 320;
            // if (lcd_height > 240) lcd_height = 240;

            // for ( U32 i = 0; !fail && i < 2; ++i )
            // {
            //     _sprites[i].setPsram(true);
            //     fail = !_sprites[i].createSprite(lcd_width, lcd_height);
            // }
            if( fail )
            {
                Serial.print("createSprite fail...");
                lgfx::delay(3000);
            }
        }
    }

    pinMode( SD_CS, OUTPUT );
    digitalWrite( SD_CS, HIGH );
    SPI.begin( SD_SCK, SD_MISO, SD_MOSI );
    SPI.setFrequency( 16000000 );
    SD.begin( SD_CS );
    audio.setPinout( I2S_BCLK, I2S_LRC, I2S_DOUT );
    audio.setVolume( 9 ); // 0...21

    // =============================================================
    hal_screen_startup_out();
    paint_color_set( COLOR_WHITE );
    paint_color_background_set( COLOR_BLACK );
    paint_font( PAINT_FONT_Generic8, PAINT_FONT_MS );
    paint_font_mastab_set( 2 );
    paint_font_mode_transparent_set( 1 );
    

    sound_play( "/0018_battery_pickup.mp3" );

    // delay(500);
    // paint_screen_clear();
    // hal_paint_screen_update();
    // paint_screen_clear();
    // hal_paint_screen_update();

// #define FILTER_K 		95. //90.
//     filt_0.window = FILTER_K;

    fd_dsp_draw_init( &fd_data );

#if UART_PROTOCOL_MAVLINK
#else
    // parser init =============================================================
    modParser_init( &parser_rx, PIK_ADR_DEFAULT, PARSER_MAX_DATA_SIZE );
    modParser_init( &parser_tx, PIK_ADR_DEFAULT, PARSER_MAX_DATA_SIZE );
#endif

    Serial.printf("\r\nINIT ALL OK!\r\n" );
}

void sendMAVLink( void )
{
    fd_dsp_calibrate_send_process();

    static U32 lastSent = 0;
    if( millis() - lastSent < 1000 ) return; // Send every second

    lastSent = millis();

    // Generate HEARTBEAT message buffer
    mavlink_message_t msg;
    U8 buf[ MAVLINK_MAX_PACKET_LEN ];

    mavlink_msg_heartbeat_pack( 1, MAV_COMP_ID_AUTOPILOT1, &msg, MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_GENERIC, MAV_MODE_FLAG_MANUAL_INPUT_ENABLED, 0, MAV_STATE_STANDBY );
    U16 len = mavlink_msg_to_send_buffer( buf, &msg );

    Serial.write( buf, len );
}

void fd_dsp_MAVLink_recive( void ) // Parse MAVLink message
{
    static mavlink_message_t msg;
    static mavlink_status_t status;

    U32 cnt_max = 4000;
    while( Serial.available() > 0 && cnt_max-- )
    {
        char tmp_char = Serial.read(); // get incoming byte
        serial_chars_cnt++;

#if UART_PROTOCOL_MAVLINK
        if( mavlink_parse_char( MAVLINK_COMM_0, tmp_char, &msg, &status ))
        {
            // switch( msg.msgid )
            // {
            // case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
            //     mavlink_rc_channels_override_t mav_com = {0};
            //     mavlink_msg_rc_channels_override_decode( &msg, &mav_com );

            //     // snr level after 1-x cascade for all
            //     fd_data.snr[0] = mav_com.chan1_raw;

            //     // snr level after 2-x cascade for all
            //     fd_data.snr[1] = mav_com.chan2_raw;

            //     // snr level after 2-x cascade
            //     fd_data.snr[2] = mav_com.chan3_raw; // for ch1
            //     fd_data.snr[3] = mav_com.chan4_raw; // for ch2
            //     fd_data.snr[4] = mav_com.chan5_raw; // for ch3
            //     fd_data.snr[5] = mav_com.chan6_raw; // for ch4

            //     // rssi after 1-x cascade
            //     fd_data.v[0] = mav_com.chan7_raw;  // for ch1
            //     fd_data.v[1] = mav_com.chan8_raw;  // for ch2
            //     fd_data.v[2] = mav_com.chan9_raw;  // for ch3
            //     fd_data.v[3] = mav_com.chan10_raw; // for ch4

            //     // rssi after 2-x cascade
            //     fd_data.v[4] = mav_com.chan11_raw; // for ch1
            //     fd_data.v[5] = mav_com.chan12_raw; // for ch2
            //     fd_data.v[6] = mav_com.chan13_raw; // for ch3
            //     fd_data.v[7] = mav_com.chan14_raw; // for ch4

            //     fd_dsp_add_new_vector( &fd_data, mul );

            //     mavlink_msg_cnt++;
            //     break;

            // // default:
            // //   err++;
            // //     //Serial.print("Received message with ID ");
            // //     //Serial.println(msg.msgid);
            // //     break;
            // }

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

                // rssi after 1-x cascade
                fd_data.v[0] = mav_fd.b1ch3; // for ch1 _constrain( fd_data.v[0], 2, 65533 ); //ch1
                fd_data.v[1] = mav_fd.b1ch1; // for ch2
                fd_data.v[2] = mav_fd.b1ch2; // for ch3
                fd_data.v[3] = mav_fd.b1ch4; // for ch4

                // rssi after 2-x cascade
                fd_data.v[4] = mav_fd.b2ch3; // for ch1
                fd_data.v[5] = mav_fd.b2ch1; // for ch2
                fd_data.v[6] = mav_fd.b2ch2; // for ch3
                fd_data.v[7] = mav_fd.b2ch4; // for ch4

                // snr level after 2-x cascade
                fd_data.snr[2] = mav_fd.snr_b1ch1; // for ch1
                fd_data.snr[3] = mav_fd.snr_b1ch2; // for ch2
                fd_data.snr[4] = mav_fd.snr_b1ch3; // for ch3
                fd_data.snr[5] = mav_fd.snr_b1ch4; // for ch4

                // snr level after 2-x cascade
                fd_data.snr[2] = mav_fd.snr_b2ch1; // for ch1
                fd_data.snr[3] = mav_fd.snr_b1ch2; // for ch2
                fd_data.snr[4] = mav_fd.snr_b2ch3; // for ch3
                fd_data.snr[5] = mav_fd.snr_b2ch4; // for ch4

                fd_data.fx[0] = mav_fd.x[0];
                fd_data.fx[1] = mav_fd.x[1];
                fd_data.fy[0] = mav_fd.y[0];
                fd_data.fy[1] = mav_fd.y[1];

//            fd_data.fx[0] = mav_fd.x[0];
//            fd_data.fx[1] = mav_fd.x[1];
//            fd_data.fy[0] = mav_fd.y[0];
//            fd_data.fy[1] = mav_fd.y[1];

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
    sec = lgfx::millis() / 1000;
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
    mainfunc();
    drawfunc();
    sendMAVLink();
    fd_dsp_MAVLink_recive();
}


#ifdef	__cplusplus
extern "C" {
#endif

COORD hal_paint_screen_width( void )
{
  return SCREEN_W;
}

COORD hal_paint_screen_height( void )
{
  return SCREEN_H;
}

RET hal_paint_init( INT mode )
{
    _sprites[0].clear();
    _sprites[1].clear();
    return RET_OK;
}

void hal_paint_pixel_set( COORD x, COORD y, COLOR color )
{
    //lcd.drawPixel( x, y, color );
    _sprites[ flip ].drawPixel( x, y, color );
}

COLOR hal_paint_pixel_get( COORD x, COORD y )
{
    if( ( x >= hal_paint_screen_width() ) ||
        ( y >= hal_paint_screen_height() ) ||
            ( x < 0 ) || ( y < 0 ) )
    {
        return COLOR_BLACK;
    }
    else
    {
        return 0;//lcd.pixel( x, y );
    }
}

void hal_paint_block( COORD x, COORD y, COORD w, COORD h, COLOR *color )
{
    COORD i, j;

    for( j = 0; j < h; j++)
    {
        for( i = 0; i < w; i++)
        {
            hal_paint_pixel_set( i + x, j + y, *color++ );
        }
    }
}

void hal_paint_block_color( COORD x, COORD y, COORD w, COORD h, COLOR color )
{
    _sprites[ flip ].fillRect( x, y, w, h, color );
    // COORD i, j;

    // for( j = 0; j < h; j++)
    // {
    //     for( i = 0; i < w; i++)
    //     {
    //         hal_paint_pixel_set( i + x, j + y, color );
    //     }
    // }
}

void hal_paint_block_transparent( COORD x, COORD y, COORD w, COORD h, COLOR *color, COLOR transparent )
{
    COORD i, j;

    for( j = 0; j < h; j++ )
    {
        for( i = 0; i < w; i++ )
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
  //lcd.fillScreen( color );
  _sprites[ flip ].fillRect(0, 0, 800, 480, color );
}

RET hal_paint_screen_update( void )
{
    //U8 *p8 = (std::uint8_t *)_sprites[flip].getBuffer();
    //lcd.pushImage( 0, 0, 800, 480, p8 );
    _sprites[ flip ].pushSprite( &lcd, 0, 0 ); // Draw sprite at coordinates 0, 0 on lcd
    //lcd.display();
    flip = flip ^ 1;
    return RET_OK;
}

#ifdef	__cplusplus
}
#endif
