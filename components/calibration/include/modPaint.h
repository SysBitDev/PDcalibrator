/*
 * @file    main paint functions
 * @author  Ht3h5793, CD45
 * @date    05.03.2014
 * @version V13.8.6
 * @brief - micro grafic library

*/

#ifndef MODPAINT_H
#define	MODPAINT_H 20200202 /* Revision ID */

#include "colors.h"
#include "board.h"
#include "common.h"

#ifndef COORD
#define COORD S32
#endif

/* Point mode type */
typedef enum {
    PAINT_PIXEL_CLR = 0,
    PAINT_PIXEL_SET = 1,
    PAINT_PIXEL_XOR = 2	// Note: PIXEL_XOR mode affects only to vram_put_point().
                    // Graph functions only transmit parameter to vram_put_point() and result of graph functions
                    // depends on it's implementation. However Line() and Bar() functions work well.
} MODPAINT_point_mode;

/** quarters (for circle)
  * II  | I
  * III | IV
  */
typedef enum  {
    PAINT_QUARTERS_I   = 0x01,
    PAINT_QUARTERS_II  = 0x02,
    PAINT_QUARTERS_III = 0x04,
    PAINT_QUARTERS_IV  = 0x08,
    PAINT_QUARTERS_ALL = 0x0F,
/*@param[in] sectors	Bits determine which sectors are drawn.
 * Bits go anti-clockwise from the 0 degree mark (y = 0, x is positive), as follows:
 * bit 0 - upper right right	  -----
 * bit 1 - upper upper right	 /2   1\
 * bit 2 - upper upper left		/3     0\
 * bit 3 - upper left  left		\4     7/
 * bit 4 - lower left  left		 \5   6/
 * bit 5 - lower lower left		  -----
 * bit 6 - lower lower right
 * bit 7 - lower left  left
 **/
} MODPAINT_QUARTERS;

typedef enum {
    PAINT_FONT_MS = 0, //monospaced
    PAINT_FONT_PP = 1, //proportional
    PAINT_FONT_TRANSPARENT

} PAINT_FONT_TYPE;


///* Font descriptor type */
//typedef struct {
//    U16 char_width;
//    U16 char_offset;
//} font_descriptor_t;
//
//
//typedef struct {
//    U16 width; // max width
//    U16 height; // height
//    U16 vSpacing; // vSpacing
//    U16 hSpacing; // hSpacing
//    U16 offset; // offset
//    U16 rezerv_1; // rezerv
//    U16 rezerv_2; // rezerv
//    unsigned char start_char;
//    unsigned char end_char;
//    font_descriptor_t *descr_array;
//    const unsigned char *font_bitmap_array;
//} font_info_t;


#pragma pack( push, 1 )
typedef struct {
    //Char  X pos   Y pos   Width   Height   Xoffset  Yoffset  Orig W   Orig H
    U16 ch;
    U16 posX;
    U16 posY;
    U8 W;
    U8 H;
    S8 offsX;
    S8 offsY;
    U8 rW;
    U8 rH;
} font_descr_t;
#pragma pack( pop )

typedef struct {
    const U32 *glyphs;
    font_descr_t *descr;
    U16 size;
    U8 maxW;
    U8 maxH;
} font_t;

typedef struct {
    COLOR fg_color; // the current foreground drawing color
    COLOR bg_color; // the current background drawing color
    U8 orientation;
    INT buf_num;

    U8 font_num;
    font_t *font;
    U8 font_type;
    COORD cX; // current cursor position
    COORD cY;
    FAST_U8 offsetX, offsetY;
    FAST_U8 W, H; //max size
    UINT maxCol, maxRow;
    FAST_U8 font_mashtab;
    FAST_U8 font_mode_transparent;
} paint_t;

typedef struct {
    COORD x;
    COORD y;
} COORDXY;


typedef struct {
    COORD x;
    COORD y;
    COORD w;
    COORD h;
} BOX2D;


typedef struct {
    FLOAT *mod;
    FLOAT *dir;
    INT size;
    COLOR color;
} vector_sprite_t;


//#pragma pack( push, 1 )
typedef struct
{
#if DRAW_2_ASCII
    char ch;
    COLOR color;
#else
    COLOR *p;
    COORD w;
    COORD h;
#endif

} sprite_t;
//#pragma pack( pop )

typedef struct {
    sprite_t sp;
    BOX2D box2D;
} _obj_t;


#ifdef	__cplusplus
extern "C" {
#endif

//================================================================

/**
 * @brief paint_width
 * @return
 */
COORD 	paint_width( void );
#define paint_width_half()   ( paint_width() >> 1 )

/**
 * @brief paint_height
 * @return
 */
COORD 	paint_height( void );
#define paint_height_half()   ( paint_height() >> 1 )

/**
 * @brief paint_init
 * @param mode
 * @return
 */
RET     paint_init( U8 mode );

/**
 * @brief paint_deinit
 * @param mode
 * @return
 */\
RET     paint_deinit( U8 mode );

/**
 * @brief paint_color_set
 * @param color
 */
void    paint_color_set( COLOR color );

/**
 * @brief paint_color_get - get current color
 * @return
 */
COLOR paint_color_get( void );

/**
 * @brief paint_color
 * @param color
 * @return
 */
COLOR paint_color( COLOR color );

/**
 * @brief paint_pixel_get
 * @param x
 * @param y
 * @return
 */
COLOR paint_pixel_get( COORD x, COORD y );

/**
 * @brief paint_color_background_set
 * @param color
 */
void    paint_color_background_set( COLOR color );

/**
 * @brief paint_color_background_get
 * @return
 */
COLOR   paint_color_background_get( void );

/**
 * @brief paint_color_background
 * @param color
 * @return
 */
COLOR   paint_color_background( COLOR color );


U8  	paint_color_get_R8( COLOR color );
U8  	paint_color_get_G8( COLOR color );
U8  	paint_color_get_B8( COLOR color );

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
U8      paint_RGB888_to_RGB8 (U8 r, U8 g, U8 b);
COLOR	paint_RGB888_to_RGB16( U8 r, U8 g, U8 b );
void  	paint_RGB16_to_RGB888 (COLOR color, U8 *pColor);
COLOR 	paint_blend_2_colors (COLOR fg, COLOR bg, U8 alpha);


/**
 * @brief paint_screen_clear
 */
void paint_screen_clear( void );

/**
 * @brief paint_screen_update
 * @return
 */
// must call hal_paint_update, if need (check hardware implementation!)
RET paint_screen_update( void );

/**
 * @brief paint_pixel
 * @param x
 * @param y
 */
void    paint_pixel( COORD x, COORD y );

/**
 * @brief paint_pixel_color
 * @param x
 * @param y
 * @param color
 */
void    paint_pixel_color( COORD x, COORD y, COLOR color );

/**
 * @brief paint_line_x
 * @param x0
 * @param y0
 * @param w
 */
void paint_line_x( COORD x0, COORD y0, COORD w );

/**
 * @brief paint_line_y
 * @param x0
 * @param y0
 * @param h
 */
void paint_line_y( COORD x0, COORD y0, COORD h );

/**
 * @brief paint_line
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 */
void paint_line( COORD x1, COORD y1, COORD x2, COORD y2 );
//TODO
void paint_line_width( COORD x1, COORD y1, COORD x2, COORD y2, U32 w );

/**
 * @brief paint_rectangle
 * @param x1
 * @param y1
 * @param w
 * @param h
 * @param filled
 */
void paint_rectangle( COORD x1, COORD y1, COORD w, COORD h, BOOL filled );

/**
 * @brief paint_rectangle_round
 * @param x
 * @param y
 * @param w
 * @param h
 * @param r
 * @param filled
 */
void paint_rectangle_round( COORD x, COORD y, COORD w, COORD h, COORD r, BOOL filled );

/**
 * @brief paint_circle
 * @param x
 * @param y
 * @param radius
 * @param corner_type
 * @param filled
 */
void CODE paint_circle( COORD x, COORD y, COORD radius, MODPAINT_QUARTERS corner_type, BOOL filled );

/**
 * @brief paint_ellipse
 * @param x
 * @param y
 * @param w
 * @param h
 * @param corner_type
 * @param filled
 */
void paint_ellipse( COORD x, COORD y, COORD w, COORD h, MODPAINT_QUARTERS corner_type, BOOL filled );

/**
 * @brief paint_triangle
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param x3
 * @param y3
 * @param filled
 */
void paint_triangle( COORD x1, COORD y1, COORD x2, COORD y2, COORD x3, COORD y3, BOOL filled );

/**
 * @brief paint_sprite_vector
 * @param sp
 * @param x
 * @param y
 * @param dir
 */
void paint_sprite_vector( vector_sprite_t *sp, COORD x, COORD y, FLOAT dir );

//void paint_bitmap_mono( COORD x, COORD y, COORD w, COORD h, const U8 *buf );


/**
 * @brief paint_bitmap_color
 * @param x - x coord
 * @param y - y coord
 * @param w - width
 * @param h - height
 * @param buf -
 */
void paint_bitmap( COORD x, COORD y, COORD w, COORD h, COLOR *buf );

/**
 * @brief paint_bitmap_transparent
 * @param x - x coord
 * @param y - y coord
 * @param w - width
 * @param h - height
 * @param buf -
 * @param transparent - color that will be transparent
 */
void paint_bitmap_transparent( COORD x, COORD y, COORD w, COORD h, COLOR *buf, COLOR transparent );

/**
 * @brief paint_sprite
 * @param x
 * @param y
 * @param sp
 */
void paint_sprite( COORD x, COORD y, const sprite_t *sp );

/**
 * @brief paint_sprite_transparent
 * @param x
 * @param y
 * @param sp
 */
void paint_sprite_transparent( COORD x, COORD y, const sprite_t *sp );


// Fonts, test =================================================================
/**
 * @brief paint_font
 * @param num
 * @param type
 */
void paint_font( U8 num, PAINT_FONT_TYPE type );
//void   	paint_setTextSize (U8 s);

/**
 * @brief paint_font_mastab_set
 * @param mashtab - from 1 (default) to 4
 */
void paint_font_mastab_set( FAST_U8 mashtab );

/**
 * @brief paint_font_mode_transparent_set
 * @param mode - 0 - no transparent, bg pixel set, 1 - bg pixel not draw
 */
void paint_font_mode_transparent_set( FAST_U8 mode );

/**
 * @brief paint_get_max_col
 * @return x-pos
 */
COORD paint_get_max_col( void );

/**
 * @brief paint_get_max_row
 * @return y-pos
 */
COORD paint_get_max_row( void );

/**
 * @brief paint_font_w_get
 * @return
 */
COORD paint_font_w_get( void );

/**
 * @brief paint_font_h_get
 * @return
 */
COORD paint_font_h_get( void );

/**
 * @brief paint_set_col
 * @param col
 * @return
 */
RET     paint_set_col( COORD col ); //x-pos

/**
 * @brief paint_set_row
 * @param row
 * @return
 */
RET     paint_set_row( COORD row ); //y-pos

/**
 * @brief paint_set_col_row
 * @param col
 * @param row
 * @return
 */
RET		paint_set_col_row( COORD col, COORD row );

/**
 * @brief paint_get_col
 * @return
 */
COORD	paint_get_col( void ); //x-pos

/**
 * @brief paint_get_row
 * @return
 */
COORD	paint_get_row( void ); //y-pos

/**
 * @brief paint_put_char_col_row
 * @param col
 * @param row
 * @param c
 */
void	paint_put_char_col_row( COORD col, COORD row, char c );

/**
 * @brief paint_put_char
 * @param c
 */
void	paint_put_char( char c );

/**
 * @brief paint_text_clear_row
 * @param row
 */
void	paint_text_clear_row( COORD row );

/**
 * @brief paint_text_col_row
 * @param col
 * @param row
 * @param str
 */
void	paint_text_col_row( COORD col, COORD row, const char *str );

/**
 * @brief paint_text
 * @param str
 */
void	paint_text( const char *str);

/**
 * @brief paint_char_xy
 * @param x
 * @param y
 * @param c
 * @return
 */
COORD	paint_char_xy( COORD x, COORD y, char c );


/**
 * @brief paint_text_xy
 * @param x
 * @param y
 * @param str
 * @return
 */
COORD	paint_text_xy( COORD x, COORD y, const char *str );


// expperimental ===============================================================
U32 paint_msg_println( const char *str );

COORD paint_INT_spaced( COORD x, COORD y, INT val, INT dig_max_num );

/**
 * @brief paint_oscill_S32 - draw experimental oscillogram
 * @param p
 * @param x
 * @param y
 * @param range_x
 * @param range_y
 * @param samples_num
 */
void paint_oscill_S32( S32 *p, COORD x, COORD y, INT range_x, INT range_y, INT samples_num );

/**
 * @brief paint_test
 */
void paint_test( void );


void hal_screen_startup_out( void );


#ifdef	__cplusplus
}
#endif

#endif	/* MODPAINT_H */
