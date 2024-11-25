/*
 * @file    color.h
 * @author  ZAY_079, HT3H5796-8
 * @date    05.03.2014
 * @version V1.0.6
 * @brief

*/

#ifndef COLORS_H
#define	COLORS_H 20200505 /* Revision ID */

#include "board.h"



#if COLOR_BI
#ifndef COLOR
//TODO
#define COLOR                			U8
#define COLOR_R_OFFSET				    2
#define COLOR_G_OFFSET				    1
#define COLOR_B_OFFSET				    0
#define COLOR_R_OFFSET_MASK				0
#define COLOR_G_OFFSET_MASK				0
#define COLOR_B_OFFSET_MASK				0
#define COLOR_R_MASK					0x0001
#define COLOR_G_MASK					0x0001
#define COLOR_B_MASK					0x0001
#endif
#endif


#if COLOR_RGB323 /** 8bit color */
#define COLOR                           U8
#define COLOR_R_OFFSET				    5
#define COLOR_G_OFFSET				    3
#define COLOR_B_OFFSET				    0
#define COLOR_R_OFFSET_MASK				(8-3)
#define COLOR_G_OFFSET_MASK				(8-2)
#define COLOR_B_OFFSET_MASK				(8-3)
#define COLOR_R_MASK					0x00E0
#define COLOR_G_MASK					0x00C0
#define COLOR_B_MASK					0x00E0
#endif // COLOR_RGB565


#if COLOR_RGB444 /** 12bit color */
#define COLOR                           U16
#define COLOR_R_OFFSET				    8
#define COLOR_G_OFFSET				    4
#define COLOR_B_OFFSET				    0
#define COLOR_R_OFFSET_MASK				(8-4)
#define COLOR_G_OFFSET_MASK				(8-4)
#define COLOR_B_OFFSET_MASK				(8-4)
#define COLOR_R_MASK					0x00F0
#define COLOR_G_MASK					0x00F0
#define COLOR_B_MASK					0x00F0
#endif // COLOR_RGB565


#if COLOR_RGB565 /** 16bit color */
#define COLOR                           U16
#define COLOR_R_OFFSET				    11
#define COLOR_G_OFFSET				    5
#define COLOR_B_OFFSET				    0
#define COLOR_R_OFFSET_MASK				(8-5)
#define COLOR_G_OFFSET_MASK				(8-6)
#define COLOR_B_OFFSET_MASK				(8-5)
#define COLOR_R_MASK					0x00F8
#define COLOR_G_MASK					0x00FC
#define COLOR_B_MASK					0x00F8
#endif // COLOR_RGB565


#if COLOR_BGR565 /** 16bit color */
#define COLOR                           U16
#define COLOR_R_OFFSET					0
#define COLOR_G_OFFSET					5
#define COLOR_B_OFFSET					11
#define COLOR_R_OFFSET_MASK				(8-5)
#define COLOR_G_OFFSET_MASK				(8-6)
#define COLOR_B_OFFSET_MASK				(8-5)
#define COLOR_R_MASK					0x00F8
#define COLOR_G_MASK					0x00FC
#define COLOR_B_MASK					0x00F8
#endif // COLOR_BGR565


#if COLOR_RGB888 /** 24bit color */
#define COLOR                           U32
#define COLOR_R_OFFSET				    16
#define COLOR_G_OFFSET				    8
#define COLOR_B_OFFSET				    0
#define COLOR_R_OFFSET_MASK				0
#define COLOR_G_OFFSET_MASK				0
#define COLOR_B_OFFSET_MASK				0
#define COLOR_R_MASK					0x000000FF
#define COLOR_G_MASK					0x000000FF
#define COLOR_B_MASK					0x000000FF
#endif // COLOR_RGB888


#if COLOR_GBR888 /** 24bit color */
#define COLOR                           U32
#define COLOR_R_OFFSET				    16
#define COLOR_G_OFFSET				    8
#define COLOR_B_OFFSET				    0
#define COLOR_R_OFFSET_MASK				0
#define COLOR_G_OFFSET_MASK				0
#define COLOR_B_OFFSET_MASK				0
#define COLOR_R_MASK					0x000000FF
#define COLOR_G_MASK					0x000000FF
#define COLOR_B_MASK					0x000000FF
#endif // COLOR_RGB888


#if ARGB8888
#endif


#if COLOR_RGB565_Qt /** 16bit color */
#define COLOR                           U32
#define COLOR_R_OFFSET				    11
#define COLOR_G_OFFSET				    5
#define COLOR_B_OFFSET				    0
#define COLOR_R_OFFSET_MASK				(8-5)
#define COLOR_G_OFFSET_MASK				(8-6)
#define COLOR_B_OFFSET_MASK				(8-5)
#define COLOR_R_MASK					0x00F8
#define COLOR_G_MASK					0x00FC
#define COLOR_B_MASK					0x00F8
#endif // COLOR_RGB565


// repack from x-bit presentation into color structure
#define color_RGB_repack(r,g,b)\
    ((COLOR)((r & COLOR_R_MASK) >> COLOR_R_OFFSET_MASK) << COLOR_R_OFFSET) |\
    ((COLOR)((g & COLOR_G_MASK) >> COLOR_G_OFFSET_MASK) << COLOR_G_OFFSET) |\
    ((COLOR)((b & COLOR_B_MASK) >> COLOR_B_OFFSET_MASK) << COLOR_B_OFFSET)

//#define color_repack(c)
//    ((COLOR)((c & COLOR_R_MASK) >> COLOR_R_OFFSET_MASK) << COLOR_R_OFFSET) |
//    ((COLOR)((c & COLOR_G_MASK) >> COLOR_G_OFFSET_MASK) << COLOR_G_OFFSET) |
//    ((COLOR)((c & COLOR_B_MASK) >> COLOR_B_OFFSET_MASK) << COLOR_B_OFFSET)


#define color_RGB16_to_RGB32(a)         (0xFF000000 | ((U32)(a & 0xF800)<<8) | ((U32)(a & 0x07E0)<<5) | ((U32)(a & 0x001F)<<3))
//#define RGB888_to_RGB32(r, g, b)           (0xFF000000 | ((U32)(r & 0xF800)<<5) | ((U32)(g & 0x07E0)<<3) | (U32)(b & 0x001F))


#define color_RGB888_to_RGB16(r,g,b)    color_RGB_repack(r,g,b)
//#define RGB888_565(r, g, b)             ((r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11))
#define _color_RGB888_to_RGB16(a)       ( ((a & 0xF80000)>>8) | ((U32)(a & 0xFC00)>>5) | ((U32)(a & 0x00F8)>>3) )


#define GRAY888_565(g)                  ((g >> 3) | ((g >> 2) << 5) | ((g >> 3) << 11))


// https://www.rapidtables.com/web/color/RGB_Color.html
typedef enum {
    COLOR_NONE			                = color_RGB_repack(0,0,0), //special colors
    COLOR_INVERSE						= color_RGB_repack(0,255,255), //special colors
    // transparent - bright green
    COLOR_TRANSPARENT					= color_RGB_repack(0,255,0),

    COLOR_BLUE_MIN                     	= color_RGB_repack(0,0,1),
    COLOR_GREEN_MIN                     = color_RGB_repack(0,1,0),
    COLOR_RED_MIN                     	= color_RGB_repack(1,0,0),

    COLOR_BLACK			                = color_RGB_repack(0,0,0),
    COLOR_WHITE			                = color_RGB_repack(255,255,255),
    COLOR_RED			                = color_RGB_repack(255,0,0),
    COLOR_LIME                          = color_RGB_repack(0,255,0),
    COLOR_BLUE			                = color_RGB_repack(0,0,255),
    COLOR_YELLOW		                = color_RGB_repack(255,255,0),
    COLOR_AQUA                          = color_RGB_repack(0,255,255), //cian
    COLOR_MAGENTA                       = color_RGB_repack(255,0,255), //fuchsia
    COLOR_SILVER                        = color_RGB_repack(192,192,192),
    COLOR_GRAY			                = color_RGB_repack(128,128,128),
    COLOR_MAROON			            = color_RGB_repack(128,0,0),
    COLOR_OLIVE                         = color_RGB_repack(128,128,0),
    COLOR_PURPLE		                = color_RGB_repack(128,0,128),
    COLOR_TEAL                          = color_RGB_repack(0,128,128),
    COLOR_NAVY                          = color_RGB_repack(0,0,128),

    COLOR_DARK_RED			            = color_RGB_repack(139,0,0),
    COLOR_BROWN                         = color_RGB_repack(165,42,42),
    COLOR_FIREBRICK                     = color_RGB_repack(178,34,34),
    COLOR_CRIMSON                       = color_RGB_repack(220,20,60),
    COLOR_TOMATO                        = color_RGB_repack(255,99,71),
    COLOR_CORAL                         = color_RGB_repack(255,127,80),
    COLOR_LIGHT_SALMON                  = color_RGB_repack(255,160,122),
    COLOR_ORANGE_RED                    = color_RGB_repack(255,69,0),
    COLOR_DARK_ORANGE                   = color_RGB_repack(255,140,0),
    COLOR_ORANGE		                = color_RGB_repack(255,165,0),
    COLOR_GOLD                          = color_RGB_repack(255,215,0),

    COLOR_KHAKI                         = color_RGB_repack(240,230,140),
    COLOR_LAWN_GREEN	                = color_RGB_repack(124,252,0),
    COLOR_GREEN_YELLOW	                = color_RGB_repack(173,255,47),
    COLOR_DARK_GREEN	                = color_RGB_repack(0,100,0),
    COLOR_GREEN			                = color_RGB_repack(0,128,0),
    COLOR_FOREST_GREEN			        = color_RGB_repack(34,139,34),
    COLOR_LIME_GREEN			        = color_RGB_repack(50,205,50),
    COLOR_SPRING_GREEN			        = color_RGB_repack(0,255,127),

    COLOR_DARK_CIAN                     = color_RGB_repack(0,139,139),
    COLOR_TURQUOISE                     = color_RGB_repack(64,224,208),
    COLOR_AQUA_MARINE                   = color_RGB_repack(127,255,212),
    COLOR_STEEL_BLUE                    = color_RGB_repack(70,130,180),
    COLOR_SKY_BLUE                      = color_RGB_repack(135,206,235),
    COLOR_MIDNIGHT_BLUE                 = color_RGB_repack(25,25,112),
    COLOR_DARK_BLUE                     = color_RGB_repack(0,0,139),
    COLOR_MEDIUM_BLUE                   = color_RGB_repack(0,0,205),
    COLOR_ROYAL_BLUE                    = color_RGB_repack(65,105,225),

    COLOR_INDIGO                        = color_RGB_repack(75,0,130),
    COLOR_DARK_MAGENTA                  = color_RGB_repack(139,0,139),
    COLOR_DARK_VIOLET                   = color_RGB_repack(148,0,211),
    COLOR_VIOLET                        = color_RGB_repack(238,130,238),
    COLOR_DEEP_PINK                     = color_RGB_repack(255,20,147),
    COLOR_HOT_PINK                      = color_RGB_repack(255,105,180),
    COLOR_LIGHT_PINK                    = color_RGB_repack(255,182,193),
    COLOR_PINK                          = color_RGB_repack(255,192,203),

    COLOR_BISQUE                        = color_RGB_repack(255,228,196),
    COLOR_CHOCOLATE                     = color_RGB_repack(210,105,30),
    COLOR_PERU                          = color_RGB_repack(205,133,63),
    COLOR_STATE_GRAY                    = color_RGB_repack(112,128,144),
    COLOR_LIGHT_STEEL_BLUE              = color_RGB_repack(176,196,222),
    COLOR_DIM_GRAY			            = color_RGB_repack(105,105,105),
    COLOR_DARK_GRAY			            = color_RGB_repack(169,169,169),
    COLOR_LIGHT_GRAY			        = color_RGB_repack(211,211,211),
    COLOR_WHITE_SMOKE			        = color_RGB_repack(245,245,245),

} COLOR_RGB_t;


#ifndef BOARD_916_AFBR_S50_vA1
#define PIXEL_OFF                       0
#define PIXEL_ON                        1
#define PIXEL_INV                       3
#define PIXEL_XOR                       3
#endif

#ifdef	__cplusplus
extern "C" {
#endif

U8 CODE color_get_R8( COLOR color );
U8 CODE color_get_G8( COLOR color );
U8 CODE color_get_B8( COLOR color );


/**
 * @brief color_get_half
 * @param color
 * @return
 */
COLOR color_get_half( COLOR color );


/**
 * @brief color_gamma_correction
 * @param val
 * @return
 */
U8 color_gamma_correction( U8 val );


/**
 * @brief color_RGB888_to_RGB565
 * @param c
 * @return
 */
U16 CODE color_RGB888_to_RGB565( U32 c );


/**
 * @brief HSV_to_RGB
    HSV to RGB
    hue        : 0..360
    saturation : 0..255
    value      : 0..255
 * @param hue
 * @param sat
 * @param val
 * @return
 */
COLOR HSV_to_RGB( INT hue, INT sat, INT val );


/**
 * @brief HSV_to_RGB888
 * @param hue
 * @param r
 * @param g
 * @param b
 */
void HSV_to_RGB888( U16 hue, U8 *r, U8 *g, U8 *b );

void color_make_rainbow_palette( COLOR *pbuf, INT size );


#ifdef	__cplusplus
}
#endif

#endif	/* COLORS_H */
