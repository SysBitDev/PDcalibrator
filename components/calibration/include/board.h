/* 
 * File:   board.h
 */

#ifndef BOARD_H
#define	BOARD_H 20240404

#include "common.h"
#include "build_defs.h"

// Specific ====================================================================
// MCU - ESP32S3

#define BOARD_ESP32_TFT7 				(1)

#define SYS_FREQ                        (84000000UL)
#define SYSTIME                         U32
#define TIC_PERIOD                      (1UL) //us


// USART =========================================================================
// #define HAL_USART1                      1
// #if HAL_USART1
// 	#define USART1_FREQ					(SYS_FREQ)
// 	#define USART1_RX_FIFO_SIZE       	(256)
// 	#define USART1_TX_FIFO_SIZE        	(256)
// 	#define USART1_RX_LED_INV           //LED_ORANGE_INV
// 	#define USART_LOGGING_NEED			0
// 	//#define USART1_TX_END				PO_USART2_DIR_L
// 	//#define USART1_SWAP_NEED			0
// #endif

// // I2C =========================================================================
// #define HAL_I2C10                		1
// #define I2C10							I2C1

// #ifdef HAL_I2C10
//     #define SCL10_L                     GPIOA->BSRR = GPIO_BSRR_BR8
//     #define SCL10_H                     GPIOA->BSRR = GPIO_BSRR_BS8
//     #define SDA10_L                     GPIOA->BSRR = GPIO_BSRR_BR9
//     #define SDA10_H                     GPIOA->BSRR = GPIO_BSRR_BS9
//     #define SCL10_IN                    ( GPIOA->IDR & GPIO_IDR_IDR_8 )
//     #define SDA10_IN                    ( GPIOA->IDR & GPIO_IDR_IDR_9 )
//     //#define I2C10_DELAY_US              30
//     #define I2C10_INIT
//     #define I2C10_DEINIT
// #endif

#define RESET_CORE_COUNT                
#define GET_CORE_COUNT                  1

//==============================================================================
// Modules settings
//==============================================================================

// modParcer ===================================================================
#define PIK_ADR_DEFAULT          		    ((U8)'#')
#define MODPARCER_CRC_CHECK             1
#define PIK_START                  		  0x0D
#define PIK_FIN                     	  0x0A
#define PARSER_MAX_DATA_SIZE        	  (1024)


// PAINT =======================================================================
#define HT3_PAINT                       1
#define LCD_800480                      1
#define SCREEN_W                        800
#define SCREEN_H                        480
#define SCREEN_ORIENTATION_NORMAL       SCREEN_ORIENTATION_0
#ifndef COORD
#define COORD S32
#endif
//#define COLOR_RGB323                    1
#define COLOR_RGB565                    1
//#define COORD                           int32_t //S32
#define SCREEN_MASHTAB				          1

// modPaint ====================================================================
#define PAINT_CHECK_BORDER				1

// fonts
//#define PAINT_FONT_x4y6                 1
#define PAINT_FONT_Generic8         	2
//#define PAINT_FONT_Arial14 				1

// paint primitives
#define PAINT_NEED_BITMAP				1
#define PAINT_NEED_TEXT					0

#if( SCREEN_W > SCREEN_H )
#define PAINT_BUF_SIZE                  SCREEN_W
#else
#define PAINT_BUF_SIZE                  SCREEN_H
#endif


// etc =========================================================================
#define LGFX_USE_V1


// DSP =========================================================================
#define DSP_FD_SAMPLING_RATE            ( 1000 * 12 )
#define DSP_FD_CHANNEL_NUM              8 /* Number of channels */
#define DSP_FD_BUF_SIZE                 3000

#define UART_PROTOCOL_MAVLINK           1 //uncomment for protocol MavLink

#define FD_DRAW                         1
#define MARK_SIZE                       15
#define GRID_SIZE                       38
#define FD_DSP_NEED_RADIUS              1
#define FD_DSP_CROSS_TYPE               3
#define FD_DSP_TRAIN_DOTS		            90


// MAIN MODE ===================================================================
#define MODE_MONITOR					1


// PAINT customs ===============================================================
//#define paint_color(c)            color_current = c
// #define paint_pixel(x,y)          lcd.drawPixel(x,y,color_current)
// #define paint_line_x(x,y,w)       lcd.drawFastHLine(x,y,w,color_current)
// #define paint_line_y(x,y,h)       lcd.drawFastVLine(x,y,h,color_current)
// #define paint_line(x1,y1,x2,y2)   lcd.drawLine(x1,y1,x2,y2,color_current)
// #define paint_circle(x,y,r,m,f)   lcd.drawCircle(x,y,r)
// #define hal_paint_screen_width()  lcd.width()
// #define hal_paint_screen_height() lcd.height()




#ifdef	__cplusplus
extern "C" {
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* BOARD_H */
