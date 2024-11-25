/*
 * @file    halPaint.h
 * @author  Ht3h5793
 * @date    29.09.2014
 * @version V6.5.7
 * @brief
 * @todo
 */

#ifndef HALSCREEN_H
#define	HALSCREEN_H 20180917 /* Revision ID */

#include "colors.h"
#include "board.h"
#include "common.h"

#ifndef COORD
#define COORD S32
#endif

/* Screen orientation (check hardware implementation!) */
typedef enum {
    SCREEN_ORIENTATION_0 	= 0,
    SCREEN_ORIENTATION_90 	= 1,
    SCREEN_ORIENTATION_180 	= 2,
    SCREEN_ORIENTATION_270 	= 3
} HALPAINT_screen_orientation_mode;


#ifdef	__cplusplus
extern "C" {
#endif
/**
 * @brief hal_paint_screen_width
 * @return
 */
COORD hal_paint_screen_width( void );


/**
 * @brief hal_paint_screen_height
 * @return
 */
COORD hal_paint_screen_height( void );


/**
 * @brief hal_paint_init
 * @param mode
 * @return
 */
RET hal_paint_init( INT mode );


/**
 * @brief hal_paint_deinit
 * @param mode
 * @return
 */
RET hal_paint_deinit( INT mode );


/**
 * @brief hal_paint_pixel_set
 * @param x
 * @param y
 * @param color
 */
void hal_paint_pixel_set( COORD x, COORD y, COLOR color );


/**
 * @brief hal_paint_pixel_get - check hardware implementation!
 * @param x
 * @param y
 * @return
 */
COLOR hal_paint_pixel_get( COORD x, COORD y );


/**
 * @brief hal_paint_block
 * @param x
 * @param y
 * @param w
 * @param h
 * @param color
 */
void hal_paint_block( COORD x, COORD y, COORD w, COORD h, COLOR *color );


/**
 * @brief hal_paint_block_color
 * @param x
 * @param y
 * @param w
 * @param h
 * @param color
 */
void hal_paint_block_color( COORD x, COORD y, COORD w, COORD h, COLOR color );


/**
 * @brief hal_paint_block_transparent
 * @param x
 * @param y
 * @param w
 * @param h
 * @param buf
 * @param transparent
 */
void hal_paint_block_transparent( COORD x, COORD y, COORD w, COORD h, COLOR *buf, COLOR transparent );


/**
 * @brief hal_paint_screen_fill
 * @param color
 */
void hal_paint_screen_fill( COLOR color );


RET hal_paint_screen_next( void );
/**
 * @brief hal_paint_screen_update
 * @return
 */
RET hal_paint_screen_update( void );


/**
 * @brief hal_paint_screen_orientation_set
 * @param orientation
 * @return
 */
RET hal_paint_screen_orientation_set( U8 orientation );


// not present for general use =================================================
/**
 * @brief hal_paint_set_aperture
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 */
void hal_paint_set_aperture( COORD x1, COORD y1, COORD x2, COORD y2 );


void hal_paint_scroll( COORD xs, COORD ys );


/**
 * @brief hal_paint_contrast_set
 * @param val
 */
void hal_paint_contrast_set( U8 val );


/**
 * @brief hal_paint_invert_mode
 * @param mode
 */
void hal_paint_invert_mode( BOOL mode );

void hal_screen_DMA_send( U8 *p, U32 size );


#ifdef	__cplusplus
}
#endif

#endif	/* HALSCREEN_H */
