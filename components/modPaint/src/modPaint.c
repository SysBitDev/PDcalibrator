#include "modPaint.h"
#include "halPaint.h"
#include "colors.h"
#include "board.h"
#include "common.h"
#include "_misc.h"
#include "xprintf.h"


#if PAINT_FONT_x4y6
    #include "font_x3y5.h"
#endif
#if PAINT_FONT_Generic8
    #include "font_Generic8.h"
#endif
#if PAINT_FONT_Arial14
    #include "font_Arial14.h"
#endif
#if PAINT_FONT_Arial_20pt
    #include "font_Arial_20pt.h"
#endif
#if PAINT_FONT_Arial_120pt
    #include "font_Arial_120pt.h"
#endif
#if PAINT_FONT_BookmanOldStyle_120pt
    #include "font_BookmanOldStyle_120pt.h"
#endif
#if PAINT_FONT_DSEG7_Modern40
    #include "DSEG7_Modern40.h"
#endif
#if PAINT_FONT_DSEG7_Courier40
    #include "Courier40.h"
#endif
#if PAINT_FONT_Nasalization_Rg32
    #include "font_Nasalization_Rg32.h"
#endif

#if( SCREEN_W > SCREEN_H )
#define PAINT_BUF_SIZE                  SCREEN_W
#else
#define PAINT_BUF_SIZE                  SCREEN_H
#endif

#ifndef paint_assert
#define paint_assert(a)
#endif

static paint_t paint;

static COLOR paint_buf[ PAINT_BUF_SIZE * 2 ]; // buff for lines/font(1-lines)


COORD CODE paint_width( void )
{
    return hal_paint_screen_width();
}

COORD CODE paint_height( void )
{
    return hal_paint_screen_height();
}

RET CODE paint_init( U8 mode )
{
    return hal_paint_init( mode );
}

RET CODE paint_deinit( U8 mode )
{
    return hal_paint_deinit( mode );
}

void CODE paint_color_set( COLOR color )
{
    paint.fg_color = color;
}

COLOR CODE paint_color_get( void )
{
    return paint.fg_color;
}

COLOR CODE paint_color( COLOR color )
{
    return paint.fg_color = color;
}

void CODE paint_color_background_set( COLOR color )
{
    paint.bg_color = color;
}

COLOR CODE paint_color_background_get( void )
{
    return paint.bg_color;
}

COLOR CODE paint_color_background( COLOR color )
{
    return paint.bg_color = color;
}

#define paint_dot_test(x,y) (( x >= hal_paint_screen_width()) || ( y >= hal_paint_screen_height()) || ( x < 0 ) || ( y < 0 ))

void  CODE paint_pixel( COORD x, COORD y )
{
#if PAINT_CHECK_BORDER
    if( ( x >= hal_paint_screen_width() ) || ( y >= hal_paint_screen_height() ))
    {
    	test_param( 1 );
    }
#endif
    hal_paint_pixel_set( x, y, paint.fg_color );
}


void CODE paint_pixel_color( COORD x, COORD y, COLOR color )
{
#if PAINT_CHECK_BORDER
    if( (x >= hal_paint_screen_width()) || (y >= hal_paint_screen_height()))
    {
    	test_param( 1 );
    }
#endif
    hal_paint_pixel_set( x, y, color );
}


COLOR CODE paint_pixel_get( COORD x, COORD y )
{
#if PAINT_CHECK_BORDER
    if( (x >= hal_paint_screen_width()) || (y >= hal_paint_screen_height()))
    {
    	test_param( 1 );
    }
#endif
    return hal_paint_pixel_get( x, y );
}


void CODE paint_screen_clear( void )
{
    hal_paint_screen_fill( paint.bg_color );
}

RET CODE paint_screen_update( void )
{
    return hal_paint_screen_update();
}

void CODE paint_line_x( COORD x, COORD y, COORD w )
{
#if PAINT_CHECK_BORDER
    if( (x >= hal_paint_screen_width()) || (y >= hal_paint_screen_height()))
    {
    	test_param( 1 );
    }
#endif
    if( w == 0 )
    {
        return;
    }
    hal_paint_block_color( x, y, w, 1, paint.fg_color );
}

void CODE paint_line_y( COORD x, COORD y, COORD h )
{
#if PAINT_CHECK_BORDER
    if( (x >= hal_paint_screen_width()) || (y >= hal_paint_screen_height()))
    {
    	test_param( 1 );
    }
#endif
    if( h == 0 )
    {
        return;
    }
    hal_paint_block_color( x, y, 1, h, paint.fg_color );
}

void CODE paint_line( COORD x1, COORD y1, COORD x2, COORD y2 )
{
#if PAINT_CHECK_BORDER
    if( (x1 >= hal_paint_screen_width()) || (y1 >= hal_paint_screen_height()))
    {
    	test_param( 1 );
    }
    if( (x2 >= hal_paint_screen_width()) || (y2 >= hal_paint_screen_height()))
    {
    	test_param( 1 );
    }
#endif

    INT dy = 0;
    INT dx = 0;
    INT stepx = 0;
    INT stepy = 0;
    INT fraction = 0;

    dy = y2 - y1;
    dx = x2 - x1;
    if( dy < 0 )
    {
        dy = -dy;
        stepy = -1;
    }
    else
        stepy = 1;

    if( dx < 0 )
    {
        dx = -dx;
        stepx = -1;
    }
    else
        stepx = 1;
    dy <<= 1;
    dx <<= 1;

    if( paint_dot_test(x1,y1) )
    {
        return;
    }
    hal_paint_pixel_set( x1, y1, paint.fg_color );

    if( dx > dy )
    {
        fraction = dy - (dx >> 1);
        while( x1 != x2 )
        {
            if( fraction >= 0 )
            {
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;
            if( paint_dot_test(x1,y1) )
            {
                return;
            }
            hal_paint_pixel_set( x1, y1, paint.fg_color );
        }
    }
    else
    {
        fraction = dx - (dy >> 1);
        while( y1 != y2 )
        {
            if( fraction >= 0 )
            {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;
            if( paint_dot_test(x1,y1) )
            {
                return;
            }
            hal_paint_pixel_set( x1, y1, paint.fg_color );
        }
    }
}


void CODE paint_line_width( COORD x1, COORD y1, COORD x2, COORD y2, U32 w )
{
#if PAINT_CHECK_BORDER
    if( (x1 >= hal_paint_screen_width()) || (y1 >= hal_paint_screen_height()))
    {
    	test_param( 1 );
    }
    if( (x2 >= hal_paint_screen_width()) || (y2 >= hal_paint_screen_height()))
    {
    	test_param( 1 );
    }
#endif

    INT dy = 0;
    INT dx = 0;
    INT stepx = 0;
    INT stepy = 0;
    INT fraction = 0;

    dy = y2 - y1;
    dx = x2 - x1;
    if( dy < 0 )
    {
        dy = -dy;
        stepy = -1;
    }
    else
        stepy = 1;

    if( dx < 0 )
    {
        dx = -dx;
        stepx = -1;
    }
    else
        stepx = 1;
    dy <<= 1;
    dx <<= 1;

    hal_paint_block_color( x1, y1, w, w, COLOR_WHITE );

    if( dx > dy )
    {
        fraction = dy - (dx >> 1);
        while( x1 != x2 )
        {
            if( fraction >= 0 )
            {
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;
            hal_paint_block_color( x1, y1, w, w, COLOR_WHITE );
        }
    }
    else
    {
        fraction = dx - (dy >> 1);
        while( y1 != y2 )
        {
            if( fraction >= 0 )
            {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;
            hal_paint_block_color( x1, y1, w, w, COLOR_WHITE );
        }
    }
}

void CODE paint_rectangle( COORD x1, COORD y1, COORD w, COORD h, BOOL filled )
{
    if( filled == TRUE )
    {
        hal_paint_block_color( x1, y1, w, h, paint.fg_color );
    }
    else
    {
        COORD x2, y2;

        x2 = x1 + ( w - 1 );
        y2 = y1 + ( h - 1 );

#if PAINT_CHECK_BORDER
        if( (x1 >= hal_paint_screen_width()) || (y1 >= hal_paint_screen_height()))
        {
            paint_assert( 1 );
        }
#endif

        paint_line_x( x1, y1, w );
        paint_line_x( x1, y2, w );
        paint_line_y( x1, y1, h );
        paint_line_y( x2, y1, h );
    }
}

void CODE paint_rectangle_round( COORD x, COORD y, COORD w, COORD h, COORD r, BOOL filled )
{
    if( filled == TRUE )
    {
        paint_rectangle( x + r, y, w - 2 * r, h, TRUE ); // @todo
        paint_circle( x+r    , y + r , r, PAINT_QUARTERS_II, TRUE ); // draw four corners
        paint_circle( x+w-r-1, y + r  , r, PAINT_QUARTERS_I, TRUE );
        paint_circle( x+w-r-1, y + h - r - 1, r, PAINT_QUARTERS_IV, TRUE );
        paint_circle( x+r    , y + h - r - 1, r, PAINT_QUARTERS_III, TRUE );
        paint_rectangle( x, y+r, r, h-2*r, TRUE );
        paint_rectangle( x+w-r-1, y+r, r, h-2*r, TRUE );
    }
    else
    {
        paint_line_x( x + r, y, w - 2 * r ); // Top
        paint_line_x( x + r, y + h - 1, w - 2 * r ); // Bottom
        paint_line_y( x, y + r, h - 2 * r ); // Left
        paint_line_y( x + w - 1, y + r , h - 2 * r ); // Right
        paint_circle( x+r    , y+r    , r, PAINT_QUARTERS_II, FALSE ); // draw four corners
        paint_circle( x+w-r-1, y+r    , r, PAINT_QUARTERS_I, FALSE );
        paint_circle( x+w-r-1, y+h - r-1, r, PAINT_QUARTERS_IV, FALSE );
        paint_circle( x+r    , y+h - r-1, r, PAINT_QUARTERS_III, FALSE );
    }
}


void CODE paint_circle( COORD x, COORD y, COORD radius, MODPAINT_QUARTERS corner_type, BOOL filled )
{
    //increment algorithm Hardeburg.
    INT xc = 0;
    INT yc;
    INT p;

    if( radius < 1 )
    {
        return;
    }

    yc = radius;
    p = 3 - (radius << 1);

    if( filled == TRUE )
    {
        while( xc <= yc )
        {
            if( corner_type & PAINT_QUARTERS_I)
            {
                paint_line_x( x, y - yc, xc );
                paint_line_x( x, y - xc, yc );
            }
            if( corner_type & PAINT_QUARTERS_IV)
            {
                paint_line_x( x, y + yc, xc );
                paint_line_x( x, y + xc, yc );
            }
            if( corner_type & PAINT_QUARTERS_III)
            {
                paint_line_x( x - xc, y + yc, xc );
                paint_line_x( x - yc, y + xc, yc );
            }
            if( corner_type & PAINT_QUARTERS_II)
            {
                paint_line_x( x - xc, y - yc, xc );
                paint_line_x( x - yc, y - xc, yc );
            }
            if( p < 0 )
            {
                p += (xc++ << 2) + 6;
            }
            else
            {
                p += ((xc++ - yc--) << 2) + 10;
            }
        }
    }
    else
    {
        while( xc <= yc )
        {
            if( corner_type & PAINT_QUARTERS_I )
            {
                hal_paint_pixel_set( x + xc, y - yc, paint.fg_color );
                hal_paint_pixel_set( x + yc, y - xc, paint.fg_color );
            }
            if( corner_type & PAINT_QUARTERS_IV )
            {
                hal_paint_pixel_set( x + xc, y + yc, paint.fg_color );
                hal_paint_pixel_set( x + yc, y + xc, paint.fg_color );
            }
            if( corner_type & PAINT_QUARTERS_III )
            {
                hal_paint_pixel_set( x - xc, y + yc, paint.fg_color );
                hal_paint_pixel_set( x - yc, y + xc, paint.fg_color );
            }
            if( corner_type & PAINT_QUARTERS_II )
            {
                hal_paint_pixel_set( x - xc, y - yc, paint.fg_color );
                hal_paint_pixel_set( x - yc, y - xc, paint.fg_color );
            }
            if( p < 0 )
            {
                p += ( xc++ << 2 ) + 6;
            }
            else
            {
                p += (( xc++ - yc-- ) << 2 ) + 10;
            }
        }
    }
}

////==============================================================================
//// Процедура рисует окружность на дисплее. x0 и y0 - координаты центра окружности
////==============================================================================
//void dispcolor_DrawCircle(int16_t x0, int16_t y0, int16_t radius, uint16_t color)
//{
//  int x = 0;
//  int y = radius;
//  int delta = 1 - 2 * radius;
//  int error = 0;
//
//  while (y >= 0)
//  {
//    dispcolor_DrawPixel(x0 + x, y0 + y, color);
//    dispcolor_DrawPixel(x0 + x, y0 - y, color);
//    dispcolor_DrawPixel(x0 - x, y0 + y, color);
//    dispcolor_DrawPixel(x0 - x, y0 - y, color);
//    error = 2 * (delta + y) - 1;
//
//    if (delta < 0 && error <= 0)
//    {
//      ++x;
//      delta += 2 * x + 1;
//      continue;
//    }
//
//    error = 2 * (delta - x) - 1;
//
//    if (delta > 0 && error > 0)
//    {
//      --y;
//      delta += 1 - 2 * y;
//      continue;
//    }
//
//    ++x;
//    delta += 2 * (x - y);
//    --y;
//  }
//}
////==============================================================================
//
//
////==============================================================================
//// Процедура рисует закрашенную окружность на дисплее. x0 и y0 - координаты центра окружности
////==============================================================================
//void dispcolor_DrawCircleFilled(int16_t x0, int16_t y0, int16_t radius, uint16_t fillcolor)
//{
//  int x = 0;
//  int y = radius;
//  int delta = 1 - 2 * radius;
//  int error = 0;
//
//  while (y >= 0)
//  {
//    dispcolor_DrawLine(x0 + x, y0 - y, x0 + x, y0 + y, fillcolor);
//    dispcolor_DrawLine(x0 - x, y0 - y, x0 - x, y0 + y, fillcolor);
//    error = 2 * (delta + y) - 1;
//
//    if (delta < 0 && error <= 0)
//    {
//      ++x;
//      delta += 2 * x + 1;
//      continue;
//    }
//
//    error = 2 * (delta - x) - 1;
//
//    if (delta > 0 && error > 0)
//    {
//      --y;
//      delta += 1 - 2 * y;
//      continue;
//    }
//
//    ++x;
//    delta += 2 * (x - y);
//    --y;
//  }
//}



void CODE paint_ellipse( COORD x, COORD y, COORD w, COORD h, MODPAINT_QUARTERS corner_type, BOOL filled )
{
    INT hh = h * h;
    INT ww = w * w;
    INT hhww = hh * ww;
    INT x0 = w;
    INT dx = 0;
    INT yy;

    if( w < 1 )
    {
        return;
    }

    if( h < 1 )
    {
        return;
    }

    if( filled == TRUE )
    {
        // do the horizontal diameter
        paint_line_x( x - w, y, w * 2 );

        //hal_paint_pixel_set( origin_x - w, origin_y, paint.fg_color );
        //hal_paint_pixel_set( origin_x + w, origin_y, paint.fg_color );

        // now do both halves at the same time, away from the diameter
        for( yy = 1; yy <= h; yy++ )
        {
            int x1 = x0 - (dx - 1);  // try slopes of dx - 1 or more
            for( ; x1 > 0; x1-- )
                if( x1 * x1 * hh + yy * yy * ww <= hhww )
                {
                    break;
                }
            dx = x0 - x1;  // current approximation of the slope
            x0 = x1;
            /*
            for( x = -x0; x <= x0; x++ )
            {
                hal_paint_pixel_set( origin_x + x, origin_y - y, paint.fg_color );
                hal_paint_pixel_set( origin_x + x, origin_y + y, paint.fg_color );
            }*/
            paint_line_x( x - x0, y - yy, x0 * 2 );
            paint_line_x( x - x0, y + yy, x0 * 2 );

            //hal_paint_pixel_set( x + x0, y - y, paint.fg_color );
            paint_line_x( x - x0, y + yy, dx );

        }
    }
    else //TODO trim or rework
    {
        INT Xc = 0, Yc = h;
        INT A2 = (INT)w*w, B2 = (INT)h*h;
        INT C1 = -(A2/4 + w % 2 + B2);
        INT C2 = -(B2/4 + h % 2 + A2);
        INT C3 = -(B2/4 + h % 2);
        INT t = -A2 * Yc;
        INT dXt = B2*Xc*2, dYt = -A2*Yc*2;
        INT dXt2 = B2*2, dYt2 = A2*2;

        while( Yc >= 0 && Xc <= w )
        {
            hal_paint_pixel_set( x + Xc, y + Yc, paint.fg_color );
            if( Xc != 0 || Yc != 0 )
              paint_pixel (x - Xc, y - Yc);
            if( Xc != 0 && Yc != 0 )
            {
                hal_paint_pixel_set( x + Xc, y - Yc, paint.fg_color );
                hal_paint_pixel_set( x - Xc, y + Yc, paint.fg_color );
            }
            if( t + Xc*B2 <= C1 || t + Yc*A2 <= C3 )
            {
                Xc++;
                dXt += dXt2;
                t   += dXt;
            }
            else if (t - Yc*A2 > C2)
            {
                Yc--;
                dYt += dYt2;
                t   += dYt;
            } else {
                Xc++;
                Yc--;
                dXt += dXt2;
                dYt += dYt2;
                t   += dXt;
                t   += dYt;
            }
        }
    }
}

void CODE paint_triangle( COORD x1, COORD y1, COORD x2, COORD y2, COORD x3, COORD y3, BOOL filled )
{
    if( filled == TRUE )
    {
        //http://compgraphics.info/2D/triangle_rasterization.php
        INT xA, xB;

        // Sort the vertices according to their y coordinate (0: y max, 1: y mid, 2:y min)
        // Упорядочиваем точки p1(x1, y1),
        // p2(x2, y2), p3(x3, y3)
        if( y2 < y1 )
        {
            _swap( y1, y2 );
            _swap( x1, x2 );
        } // точки p1, p2 упорядочены
        if( y3 < y1 )
        {
            _swap( y1, y3 );
            _swap( x1, x3 );
        } // точки p1, p3 упорядочены
        // теперь p1 самая верхняя
        // осталось упорядочить p2 и p3
        if( y2 > y3 )
        {
            _swap( y2, y3 );
            _swap( x2, x3 );
        }

        for( INT sy = y1; sy <= y3; sy++ )
        {
            xA = x1 + (sy - y1) * (x3 - x1) / (y3 - y1);
            if( sy < y2 )
            {
                xB = x1 + (sy - y1) * (x2 - x1) / (y2 - y1);
            }
            else
            {
                if (y3 == y2)
                    xB = x2;
                else
                    xB = x2 + (sy - y2) * (x3 - x2) / (y3 - y2);
            }
            if( xA > xB )
            {
                _swap( xA, xB );
            }
            paint_line( xA, sy, xB, sy );
        }
    }
    else
    {
        paint_line( x1, y1, x2, y2 );
        paint_line( x2, y2, x3, y3 );
        paint_line( x1, y1, x3, y3 );
    }
}

void paint_sprite_vector( vector_sprite_t *sp, COORD x, COORD y, FLOAT dir )
{
    FLOAT x_A, y_A, x_B, y_B, mod_A, mod_B, dir_A, dir_B;
    INT i;

    if( sp != NULL )
    {
        if( sp->size > 1 )
        {
            paint_color( sp->color );
            for( i = 0; i < sp->size - 1; i++ )
            {
                dir_A = dir_correct( dir + sp->dir[ i ] );
                mod_A = sp->mod[ i ];
                dir_B = dir_correct( dir + sp->dir[ i + 1 ] );
                mod_B = sp->mod[ i + 1 ];
                x_A = x + dir_vect_x( dir_A, mod_A );
                y_A = y + dir_vect_y( dir_A, mod_A );
                x_B = x + dir_vect_x( dir_B, mod_B );
                y_B = y + dir_vect_y( dir_B, mod_B );
                paint_line( x_A, y_A, x_B, y_B );
            }
//            //connect end to start
//            dir_A = dir_correct( dir + sp->dir[ i ] );
//            mod_A = sp->mod[ i ];
//            dir_B = dir_correct( dir + sp->dir[ 0 ] );
//            mod_B = sp->mod[ 0 ];
//            x_A = x + dir_vect_x( dir_A, mod_A );
//            y_A = y + dir_vect_y( dir_A, mod_A );
//            x_B = x + dir_vect_x( dir_B, mod_B );
//            y_B = y + dir_vect_y( dir_B, mod_B );
//            paint_line( x_A, y_A, x_B, y_B );
        }
    }
}

void CODE paint_bitmap( COORD x, COORD y, COORD w, COORD h, COLOR *buf )
{
#if PAINT_CHECK_BORDER
    if( (x >= hal_paint_screen_width()) || (y >= hal_paint_screen_height()))
    {
        paint_assert( 1 );
    }
    if( (x < 0) || (y < 0) || (w < 0) || (h < 0) )
    {
        paint_assert( 1 );
    }
    if( (x+(w-1) >= hal_paint_screen_width()) || (y+(h-1) >= hal_paint_screen_height()))
    {
        paint_assert( 1 );
    }
#endif
#if COLOR_BI
    FAST_S8 i, j, m;
    for( j = 0; j < h; j++ )
    {
        for( i = 0; i < w; i++ )
        {
            char ch = *buf++;
            for( m = 0; m < 8; m++ )
            {
                if( ch & 0x80 )
                {
                    paint_buf[ i * 8 + m ] = paint.fg_color;
                }
                else
                {
                    paint_buf[ i * 8 + m ] = paint.bg_color;
                }
                ch = ch << 1;
            }
        }
        hal_paint_block( x, y + j, w * 8, 1, &paint_buf[0] );
    }
#else
    hal_paint_block( x, y, w, h, buf );
#endif
}

void CODE paint_bitmap_transparent( COORD x, COORD y, COORD w, COORD h, COLOR *buf, COLOR transparent )
{
#if PAINT_CHECK_BORDER
    if( (x >= hal_paint_screen_width()) || (y >= hal_paint_screen_height()))
    {
        paint_assert( 1 );
    }
    if( (x < 0) || (y < 0) || (w < 0) || (h < 0) )
    {
        paint_assert( 1 );
    }
    if( (x+(w-1) >= hal_paint_screen_width()) || (y+(h-1) >= hal_paint_screen_height()))
    {
        paint_assert( 1 );
    }
#endif
    hal_paint_block_transparent( x, y, w, h, buf, transparent );
}

void CODE paint_sprite( COORD x, COORD y, const sprite_t *sp )
{
#if PAINT_CHECK_BORDER
    if( (x >= hal_paint_screen_width()) || (y >= hal_paint_screen_height()))
    {
        paint_assert( 1 );
    }
    if( (x < 0) || (y < 0) || (sp->w < 0) || (sp->h < 0) )
    {
        paint_assert( 1 );
    }
    if( (x+(sp->w-1) >= hal_paint_screen_width()) || (y+(sp->h-1) >= hal_paint_screen_height()))
    {
        paint_assert( 1 );
    }
#endif
#if COLOR_BI
    INT i, j, m;
    U8 *ptpm8 = (U8*)(sp->h);
    for( j = 0; j < sp->h; j++ )
    {
        for( i = 0; i < sp->w; i++ )
        {
            char ch = *(ptpm8++);
            for( m = 0; m < 8; m++ )
            {
                if( ch & 0x80 )
                {
                    paint_buf[ i * 8 + m ] = paint.fg_color;
                }
                else
                {
                    paint_buf[ i * 8 + m ] = paint.bg_color;
                }
                ch = ch << 1;
            }
        }
        hal_paint_block( x, y + j, sp->w * 8, 1, &paint_buf[0] );
    }
#else
    hal_paint_block( x, y, sp->w, sp->h, sp->p );
#endif
}

void CODE paint_sprite_transparent( COORD x, COORD y, const sprite_t *sp )
{
#if PAINT_CHECK_BORDER
    if( (x >= hal_paint_screen_width()) || (y >= hal_paint_screen_height()))
    {
        paint_assert( 1 );
    }
    if( (x < 0) || (y < 0) || (sp->w < 0) || (sp->h < 0) )
    {
        paint_assert( 1 );
    }
    if( (x+(sp->w-1) >= hal_paint_screen_width()) || (y+(sp->h-1) >= hal_paint_screen_height()))
    {
        paint_assert( 1 );
    }
#endif
    hal_paint_block_transparent( x, y, sp->w, sp->h, sp->p, COLOR_TRANSPARENT );
}

// FONTS =======================================================================
void CODE paint_font( U8 num, PAINT_FONT_TYPE type )
{
    paint.font_num = num;

    switch( paint.font_num )
    {
#if PAINT_FONT_x4y6
    case PAINT_FONT_x4y6:
        break;
#endif
#if PAINT_FONT_Generic8
    case PAINT_FONT_Generic8:
        paint.font = (font_t *)&font_Generic8;
        break;
#endif
#if PAINT_FONT_Arial14
    case PAINT_FONT_Arial14:
        paint.font = (font_t *)&font_Arial14;
        break;
#endif
#if PAINT_FONT_Arial_20pt
    case PAINT_FONT_Arial_20pt:
        paint.font.pFontInfo = (font_info_t *)Arial_20ptFontInfo;
        break;
#endif
#if PAINT_FONT_Arial_120pt
    case PAINT_FONT_Arial_120pt:
        paint.font.pFontInfo = (font_info_t *)Arial_120ptFontInfo;
        break;
#endif
#if PAINT_FONT_DSEG7_Modern40
    case PAINT_FONT_DSEG7_Modern40:
        paint.font = (font_t *)&DSEG7_Modern40;
        break;
#endif
/** ADD NEW FONT HERE START */
#if PAINT_FONT_DSEG7_Consolas40
    case PAINT_FONT_DSEG7_Consolas40:
        paint.font = (font_t *)&Consolas40;
        break;
#endif
#if PAINT_FONT_DSEG7_Courier40
    case PAINT_FONT_DSEG7_Courier40:
        paint.font = (font_t *)&Courier40;
        break;
#endif

#if PAINT_FONT_Nasalization_Rg32
    case PAINT_FONT_Nasalization_Rg32:
        paint.font = (font_t *)&font_Nasalization_Rg32;
        break;
#endif

/** ADD NEW FONT HERE END */
    default: while(1); break;
    }

    //paint_assert( NULL == paint.font );

    paint.buf_num = 0;

    paint.W = paint.font->maxW;
    paint.H = paint.font->maxH + 2;

    paint.maxCol = ( paint_width() / paint.W );
    paint.maxRow = ( paint_height() / ( paint.H + 0) );
    // correction to center
    paint.offsetX = ( paint_width() -  paint.maxCol * paint.W ) / 2;
    paint.offsetY = ( paint_height() - paint.maxRow * paint.H ) / 2;

    paint.font_type = type;
    paint_font_mastab_set( 1 );
    paint_font_mode_transparent_set( 0 );
//
//#if( SCREEN_H > 200 )//TODO
//    paint.maxCol = ( paint_width() / paint.W );
//    paint.maxRow = ( paint_height() / (paint.H+4) );
//    // correction to center
//    paint.offsetX = ( paint_width() -  paint.maxCol * paint.W ) / 2;
//    paint.offsetY = ( paint_height() - paint.maxRow * (paint.H+4) ) / 2;
//
//#endif
    paint_set_col_row( 0, 0 );
}

void paint_font_mastab_set( FAST_U8 mashtab )
{
    paint_assert( 0 == mashtab );
    paint.font_mashtab = mashtab;
}

void paint_font_mode_transparent_set( FAST_U8 mode )
{
    paint.font_mode_transparent = mode;
}

COORD CODE paint_char_xy( COORD x, COORD y, char c )
{
    U8 ch;
    U32 smesh;
    U32 i;
    U32 n, xx, yy, xxx, yyy;
    font_descr_t *fdescr = NULL;
    INT tmp_buf_cnt;

    if( PAINT_FONT_Generic8 == paint.font_num ) //for old 8bit dev.
    {
        ch = (U8)c;
        // todo remove this check? add new sinbols maybe
        if( 192 <= (U8)ch )
        {
            ch = (U8)(ch - ((128-32) - 6));
        }
        else
        {
            ch = (U8)ch - 32;
        }
        tmp_buf_cnt = paint.buf_num;
        smesh = (U32)ch * 5;

//        for( xx = 0; xx < 5; xx++ ) // every column of the character
//        {
//            tmp_buf_cnt = paint.buf_num;
//            ch = font_Generic8_glyphs[ smesh + xx ];
//            for( yy = 0; yy < 7 ; yy++ ) // rows...
//            {
//                if( ch & 0x01 )
//                {
//                    paint_buf[ tmp_buf_cnt ] = paint.fg_color; //[ xx * 7 + yy ]
//                }
//                else
//                {
//                    paint_buf[ tmp_buf_cnt ] = paint.bg_color;
//                }
//                tmp_buf_cnt++;
//                ch = ch >> 1;
//            }
//            hal_paint_block( x + xx, y, 1, 7, &paint_buf[ paint.buf_num ] );
//            paint.buf_num ^= 0x8;
//        }
        for( xx = 0; xx < 5; xx++ ) // every column of the character
        {
            ch = font_Generic8_glyphs[ smesh + xx ];
            for( yy = 0; yy < 7 * paint.font_mashtab; ) // rows...
            {
                COLOR c;
                if( ch & 0x01 )
                {
                    c = paint.fg_color;
                }
                else
                {
                    c = paint.bg_color;
                }
                U32 xp = xx * paint.font_mashtab;
                for( yyy = 0; yyy < paint.font_mashtab; yyy++ )
                {
                    for( xxx = 0; xxx < paint.font_mashtab; xxx++ )
                    {
                        if( 0 == paint.font_mode_transparent || c != paint.bg_color )
                            hal_paint_pixel_set( x + xp + xxx, y + yy, c );
                    }
                    yy++;
                }
                ch = ch >> 1;
            }
        }

    }
    else
    {
        for( n = 0; n < paint.font->size; n++ ) //find our symbol
        {
            fdescr = (font_descr_t *)&paint.font->descr[ n ];
            if( fdescr->ch == c ) //if found
            {
                // Copy to temporary ptr
                for( xx = 0; xx < fdescr->W; xx++ ) // Width of character
                {
                    tmp_buf_cnt = paint.buf_num;
                    xxx = xx + fdescr->posX;
                    for( yy = 0; yy < fdescr->H; yy++ ) // Height of character
                    {
                        yyy = yy + fdescr->posY;// + ( fdescr->fdescr->H);
                        i = paint.font->glyphs[ 256 * ( yyy / 32 ) + xxx ]; //256 - bit in width glyphs array, not change!
                        if( i & (U32)( (U32)1 << ( (U32)yyy & (U32)0x1F ) ) )
                        {
                            paint_buf[ tmp_buf_cnt ] = paint.fg_color;
                        }
                        else
                        {
                            paint_buf[ tmp_buf_cnt ] = paint.bg_color;
                        }
                        tmp_buf_cnt++;
                    }
                    for( ; yy < fdescr->rH; yy++ ) // Height of character
                    {
                        paint_buf[ tmp_buf_cnt ] = paint.bg_color;
                        tmp_buf_cnt++;
                    }
                    hal_paint_block( x + xx, ( c == '_' ) ? y+5 : y,
                    		1, fdescr->rH, &paint_buf[ paint.buf_num ] ); //send to
                    paint.buf_num ^= 0x80;
                }
                //space
                for( yy = 0; yy < fdescr->rH; yy++ ) // Height of character
                {
                    paint_buf[ yy ] = paint.bg_color;
                }
                for( ; xx < paint.W; xx++ ) // draw null
                {
                    hal_paint_block( x + xx, y, 1, fdescr->rH, &paint_buf[0] ); //send to
                    paint.buf_num ^= 0x80;
                }
                break;
            }
//            if( fdescr->ch != c && n == (paint.font->size -1) )
//            {
//            }
        }
        //todo char not found

    }
    // return coord for next symbol
    if( PAINT_FONT_PP & paint.font_type )
    {
        return ( x + fdescr->W + 1 );
    }
    else
    {
        return ( x + paint_font_w_get() );
    }
}

COORD CODE paint_text_xy( COORD x, COORD y, const char *str )
{
    while( '\0' != *str )
    {
        x = paint_char_xy( x, y, *str++ );
    }
    return x;
}

COORD CODE paint_get_max_col( void )
{
    return paint.maxCol;
}

COORD CODE paint_get_max_row( void )
{
    return paint.maxRow;
}

COORD CODE paint_font_w_get( void )
{
    return paint.W * paint.font_mashtab;
}

COORD CODE paint_font_h_get( void )
{
    return paint.H * paint.font_mashtab;
}

RET CODE paint_set_col( COORD col )
{
    if( col < paint_get_max_col() )
    {
        paint.cX = paint.offsetX + ( paint.W * col );
        return RET_OK;
    }
    else
        return RET_ERROR;
}

RET CODE paint_set_row( COORD row )
{
    if( row < paint_get_max_row() )
    {
        paint.cY = paint.offsetY + ( paint.H * row );
        return RET_OK;
    }
    else
    {
        return RET_ERROR;
    }
}

RET CODE paint_set_col_row( COORD col, COORD row )
{
    if ((col < paint_get_max_col()) ||
         (row < paint_get_max_row()))
    {
        paint.cX = paint.offsetX + ( paint.W * col );
        paint.cY = paint.offsetY + ( paint.H * row );
        return RET_OK;
    }
    else
    {
        return RET_ERROR;
    }
}

COORD CODE paint_get_col( void )
{
    return ( paint.cX - paint.offsetX ) / paint.W;
}

COORD CODE paint_get_row( void )
{
    return ( paint.cY - paint.offsetY ) / paint.H;
}

void CODE paint_put_char( char c )
{
    paint_char_xy( paint.cX, paint.cY, c );
}

void CODE paint_text( const char *str )
{
    COORD x, y;

    x = paint_get_col();
    y = paint_get_row();
    while( '\0' != *str )
    {
        if( '\r' == *str )
        {
            x = 0;
        }
        else if ( '\n' == *str )
        {
            y++;
        }
        else
        {
            paint_set_col_row( x, y );
            paint_put_char( *str );
            if( ++x >= paint_get_max_col() )
            {
                x = 0;
                y++;
            }
        }
        str++;
    }
}

void CODE paint_put_char_col_row( COORD col, COORD row, char c )
{
    paint_set_col_row( col, row );
    paint_put_char( c );
}

void CODE paint_text_col_row( COORD x, COORD y, const char *str )
{
    paint_set_col_row( x, y );
    paint_text( str );
}

void CODE paint_text_clear_row( COORD row )
{
    COORD i = paint_get_max_col();

    while( i-- > 0 )
    {
        paint_set_col_row ( i, row );
        paint_put_char( ' ' );
    }
}

#include <string.h>

#ifndef LOG_USES_COLORS
#define LOG_USES_COLORS 		0
#endif
#ifndef LOG_COLOR_DEFAULT
#define LOG_COLOR_DEFAULT 		COLOR_GREEN
#endif
#ifndef LOG_WARNING_COLOR
#define LOG_WARNING_COLOR 		COLOR_ORANGE
#endif
#ifndef LOG_ERROR_COLOR
#define LOG_ERROR_COLOR 		COLOR_RED
#endif


U32 paint_msg_println( const char *str )
{
	static U8 show_message_on_LCD_cnt = 0;

#if LOG_USES_COLORS
    if( NULL != strstr( str, "ERROR" ) )
    {
        paint_color_set( LOG_ERROR_COLOR );
    }
    else if( NULL != strstr( str, "WARNING" ) )
    {
        paint_color_set( LOG_WARNING_COLOR );
    }
    else
    {
        paint_color_set( LOG_COLOR_DEFAULT );
    }
#endif

    paint_text_col_row( 0, show_message_on_LCD_cnt, str );

    if( show_message_on_LCD_cnt < 0 )
    {
        show_message_on_LCD_cnt = 0;
    }
    if( ++show_message_on_LCD_cnt >= (paint_get_max_row()) )
    {
        show_message_on_LCD_cnt = 0;
    }

    return show_message_on_LCD_cnt;
}

COORD paint_INT_spaced( COORD x, COORD y, INT val, INT dig_max_num )
{
	INT i;
	COORD xn = x + dig_max_num * 6 + ( dig_max_num / 3 ) * 2;
	x = xn;
	CHAR space = 0;
	for( i = 0; i < dig_max_num; i++ )
	{
		CHAR a = val % 10 + '0';
		if( space ) a = space;
		paint_char_xy( x, y, a );
		x = x - 6;
		if(( i == 2 ) || ( i == 5 ) || ( i == 8 ))
		{
			x = x - 2;
		}
		val = val / 10;
		if( 0 == val ) space = ' ';
	}

	return xn + 6;
}

void paint_oscill_S32( S32 *p, COORD x, COORD y, INT range_x, INT range_y, INT range_x_max )
{
    COORD y0, y1;

    if( x >= range_x_max )
        x = range_x_max -1;

    if( range_x > 0 )
    {
        for( COORD i = 0; i < SCREEN_W; i+= 1 )
        {
            //if( (x + i) >= SCREEN_W ) break;
            //if( (x + i) >= SAMPLES_NUM ) break;

            if( ( i * range_x ) >= SCREEN_W ) break;
            if( (( i + 1 ) * range_x ) >= SCREEN_W ) break;

            y0 = _abs( y - p[ x + i ] / range_y );
            y0 = _constrain( y0, 0, ( SCREEN_H -1 ));

            y1 = _abs( y - p[ x + i + 1 ] / range_y );
            y1 = _constrain( y1, 0, ( SCREEN_H -1 ));

            paint_line( i * range_x, y0, ( i + 1 ) * range_x, y1 );
//            y0 = _abs( y - p[ x + i ] / range_y );
//            if( y0 > ( SCREEN_H -1) ) y0 = SCREEN_H -1;
//            y1 = _abs( y - y0 - p[ x + i + 1 ] / range_y );
//            if( y1 > ( SCREEN_H -1) ) y1 = SCREEN_H -1;
//            paint_line_y( i * range_x, y0, y1 );
        }
    }
    if( range_x < 0 )
    {
        range_x = range_x * -1;
        for( COORD i = 0; i < range_x_max; i++ )
        {
            if( (x + i * range_x) >= SCREEN_W ) break;
            //if( (x + i * range_x) >= SAMPLES_NUM ) break;

            y0 = _abs( y - p[ x + i * range_x ] / range_y );
            if( y0 < 0 ) y1 = 0;
            if( y0 > ( SCREEN_H -1) ) y0 = SCREEN_H -1;
            y1 = _abs( y - p[ x + ( i + 1) * range_x ] / range_y );
            if( y1 < 0 ) y1 = 0;
            if( y1 > ( SCREEN_H -1) ) y1 = SCREEN_H -1;
            paint_line( i, y0, i + 1, y1 );
//            y0 = _abs( y - p[ x + i * range_x ] / range_y );
//            if( y0 > ( SCREEN_H -1) ) y0 = SCREEN_H -1;
//            y1 = _abs( y - y0 - p[ x + i * range_x + 1 ] / range_y );
//            if( y1 > ( SCREEN_H -1) ) y1 = SCREEN_H -1;
//            paint_line_y( i, y0, y1 );
        }
    }
}


extern const sprite_t sp_mushroom_1;

/*
int r=1;  double q=1, n=10;
#define m  33000
double knx[ m ],  kny[ m ];
void Traf( double xA, double yA, double xB, double yB, int r)
{
    knx[r + 4] = xA;   kny[r + 4] = yA;
    knx[r + 3] = xB;   kny[r + 3] = yB;
    double xC = knx[r] = xA + yA - yB;
    double yC = kny[r] = yA + xB - xA;
    double xD = knx[r + 2] = xB + yA - yB;
    double yD = kny[r + 2] = yB + xB - xA;
    knx[r + 1] = (xC + xD*q*q + (yC - yD)*q)/(1 + q*q);
    kny[r + 1] = (yC + yD*q*q + (xD - xC)*q)/(1 + q*q);
}


void Fract()
{
    Traf( 120 -0.15, 44 -0.5, 120 - 0.15, 44 -0.5,  0 );
    for (int i = 0;  i < 5*(pow(2, n) - 1);  i += 5)
    {
        Traf(knx[i], kny[i], knx[i+1], kny[i+1], 5*r);
        Traf(knx[i+1], kny[i+1], knx[i+2], kny[i+2], 5*(r+1));
        r = r + 2;
    }
}

void Draw()
{

   paint_line(knx[3], kny[3], knx[4], kny[4]);
   for (int i=0; i < 5*r; i+=5 )
    {
        paint_line(knx[i], kny[i], knx[i+1], kny[i+1]);
        paint_line(knx[i], kny[i], knx[i+2], kny[i+2]);
        paint_line(knx[i+1], kny[i+1], knx[i+2], kny[i+2]);
        paint_line(knx[i+2], kny[i+2], knx[i+3], kny[i+3]);
        paint_line(knx[i+4], kny[i+4], knx[i], kny[i]);
    }
}
*/

void paint_test( void )
{
    // Generated by Qt image converter v0.1
    // Dimensions in pixels : 16 x 16
    // Size for 16bit color : 512 bytes
    // Size for 24bit color : 1024 bytes
    // Time image generated: 2020-04-13 07:16:36
    static const COLOR sp_mushroom_1_map[] = {
    #if COLOR_RGB565
        0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0,
        0x07E0, 0x07E0, 0x07E0, 0x0000, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x07E0, 0x07E0, 0x07E0,
        0x07E0, 0x07E0, 0x0000, 0xFFFF, 0xFFFF, 0xF800, 0xF800, 0xF800, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x07E0, 0x07E0,
        0x07E0, 0x0000, 0xFFFF, 0xFFFF, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x07E0,
        0x07E0, 0x0000, 0xFFFF, 0xF800, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF800, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x07E0,
        0x0000, 0xF800, 0xF800, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000,
        0x0000, 0xF800, 0xF800, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF800, 0xF800, 0xF800, 0xFFFF, 0xF800, 0x0000,
        0x0000, 0xFFFF, 0xF800, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF800, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000,
        0x0000, 0xFFFF, 0xFFFF, 0xF800, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF800, 0xF800, 0xF800, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000,
        0x0000, 0xFFFF, 0xFFFF, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xFFFF, 0xF800, 0x0000,
        0x0000, 0xFFFF, 0xF800, 0xF800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF800, 0xF800, 0xF800, 0x0000,
        0x07E0, 0x0000, 0x0000, 0x0000, 0xFE41, 0xFE41, 0x0000, 0xFE41, 0xFE41, 0x0000, 0xFE41, 0xFE41, 0x0000, 0x0000, 0x0000, 0x07E0,
        0x07E0, 0x07E0, 0x0000, 0xFE41, 0xFE41, 0xFE41, 0x0000, 0xFE41, 0xFE41, 0x0000, 0xFE41, 0xFE41, 0xFE41, 0x0000, 0x07E0, 0x07E0,
        0x07E0, 0x07E0, 0x0000, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0x0000, 0x07E0, 0x07E0,
        0x07E0, 0x07E0, 0x07E0, 0x0000, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0xFE41, 0x0000, 0x07E0, 0x07E0, 0x07E0,
        0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x07E0, 0x07E0, 0x07E0, 0x07E0,
    #endif
    #if COLOR_RGB888
        0x0000FF00, 0x0000FF00, 0x0000FF00, 0x0000FF00, 0x0000FF00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000FF00, 0x0000FF00, 0x0000FF00, 0x0000FF00, 0x0000FF00,
        0x0000FF00, 0x0000FF00, 0x0000FF00, 0x00000000, 0x00000000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x00000000, 0x0000FF00, 0x0000FF00, 0x0000FF00,
        0x0000FF00, 0x0000FF00, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x0000FF00, 0x0000FF00,
        0x0000FF00, 0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x0000FF00,
        0x0000FF00, 0x00000000, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000, 0x0000FF00,
        0x00000000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00000000,
        0x00000000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FF0000, 0x00000000,
        0x00000000, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000,
        0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00000000,
        0x00000000, 0x00FFFFFF, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FFFFFF, 0x00FF0000, 0x00000000,
        0x00000000, 0x00FFFFFF, 0x00FF0000, 0x00FF0000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00000000,
        0x0000FF00, 0x00000000, 0x00000000, 0x00000000, 0x00FFC90E, 0x00FFC90E, 0x00000000, 0x00FFC90E, 0x00FFC90E, 0x00000000, 0x00FFC90E, 0x00FFC90E, 0x00000000, 0x00000000, 0x00000000, 0x0000FF00,
        0x0000FF00, 0x0000FF00, 0x00000000, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00000000, 0x00FFC90E, 0x00FFC90E, 0x00000000, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00000000, 0x0000FF00, 0x0000FF00,
        0x0000FF00, 0x0000FF00, 0x00000000, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00000000, 0x0000FF00, 0x0000FF00,
        0x0000FF00, 0x0000FF00, 0x0000FF00, 0x00000000, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00FFC90E, 0x00000000, 0x0000FF00, 0x0000FF00, 0x0000FF00,
        0x0000FF00, 0x0000FF00, 0x0000FF00, 0x0000FF00, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0000FF00, 0x0000FF00, 0x0000FF00, 0x0000FF00,
    #endif
    };

    const sprite_t sp_mushroom_1 = {
        .p = (COLOR *)sp_mushroom_1_map,
        .w = 16,
        .h = 16
    };

    static INT test_num = 0;
#define CASE_TEST_NEXT   test_num = __LINE__; break; \
    case __LINE__

    COORD tmp_coord;
    char str[ 128 ];
    COORD w, h;
    //COLOR color;

    w = paint_width();
    h = paint_height();

    switch( test_num++ )
    {
    case 0: // init
        paint_font( PAINT_FONT_Generic8, PAINT_FONT_MS );

        paint_color_set( -1 );
        paint_color_background_set( 0 );
        paint_screen_clear();

    CASE_TEST_NEXT: //test paint pixel
        paint_screen_clear();
        //triforcer
        paint_pixel( 2, 2 );
        paint_pixel_color( 3, 3, COLOR_WHITE );

    CASE_TEST_NEXT: //test paint line
        paint_screen_clear();
        paint_line( 1, 1, w - 1, h - 1 );
        paint_line( 1, h - 1, w - 1, 1 );
        paint_line_x( 1, 8, w - 1 );
        paint_line_y( 8, 1, h - 1 );

    CASE_TEST_NEXT: // test paint rectangle
        paint_screen_clear();
        paint_rectangle( 0, 0, w - 1, h - 1, FALSE );
        paint_rectangle( 2, 2, 8, 8, TRUE );

    CASE_TEST_NEXT: // test paint round rectangle
        paint_screen_clear();
        paint_rectangle_round( 0, 0, w / 2 - 1, h / 2 - 1, 5, FALSE );
        paint_rectangle_round( 2, 2, 18, 18, 5, TRUE );

    CASE_TEST_NEXT: // test triangle paint
        paint_screen_clear();
        paint_triangle( w / 2 , 1, (w / 4 )* 3, h / 2,  w / 4, h / 2, FALSE );
        paint_triangle( w - 1, h - 1, w / 2, h - 1, ( w / 4 ) * 3, h / 2, FALSE );
        paint_triangle( 1, h - 1, w / 4, h / 2, w / 2, h - 1, FALSE );

    CASE_TEST_NEXT: // test triangle paint
        paint_screen_clear();
        //triforcer
        paint_triangle( w / 2 , 1, (w / 4 )* 3, h / 2, w / 4, h / 2, TRUE );
        paint_triangle( w - 1, h - 1,  w / 2, h - 1, ( w / 4 ) * 3, h / 2, TRUE );
        paint_triangle( 1, h - 1, w / 4, h / 2, w / 2, h - 1, TRUE );

    CASE_TEST_NEXT: // test circle paint
        paint_screen_clear();
        paint_circle( w / 2 , h / 2, h / 2, PAINT_QUARTERS_I, FALSE );
        paint_circle( w / 2 , h / 2, h / 2 - 2, PAINT_QUARTERS_II, FALSE );
        paint_circle( w / 2 , h / 2, h / 2 - 4, PAINT_QUARTERS_III, FALSE );
        paint_circle( w / 2 , h / 2, h / 2 - 6, PAINT_QUARTERS_IV, FALSE );
        paint_circle( w / 2 , h / 2, h / 2 - 8, PAINT_QUARTERS_ALL, FALSE );
        paint_circle( w / 2 , h / 2, h / 2 - 10, PAINT_QUARTERS_ALL, TRUE );

    CASE_TEST_NEXT: // test ellipse paint
        paint_screen_clear();
        paint_ellipse( w / 2 , h / 2, w / 2, h / 2, PAINT_QUARTERS_ALL, FALSE  );
        paint_ellipse( w / 2 , h / 2, w / 3, h / 3, PAINT_QUARTERS_ALL, TRUE );

    CASE_TEST_NEXT:
        paint_screen_clear();
        paint_bitmap( 1, 1, sp_mushroom_1.w, sp_mushroom_1.h, (COLOR *)sp_mushroom_1.p );

    CASE_TEST_NEXT:
        paint_screen_clear();
        paint_bitmap_transparent( 1, 1, sp_mushroom_1.h, sp_mushroom_1.h, (COLOR *)sp_mushroom_1.p, COLOR_TRANSPARENT );

    CASE_TEST_NEXT:
        paint_screen_clear();
        tmp_coord = 0;
        paint_text_xy( 1, tmp_coord, " !\"#$%&'()*+,-./0123456789" );
        tmp_coord += 8;
        paint_text_xy( 1, tmp_coord, ":;<=>?@[\\]^_`{|}~" );
        tmp_coord += 8;
        paint_text_xy( 1, tmp_coord, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
        tmp_coord += 8;
        paint_text_xy( 1, tmp_coord, "abcdefghijklmnopqrstuvwxyz" );

    CASE_TEST_NEXT:
        paint_screen_clear();
        tmp_coord = 0;
        paint_text_col_row( 0, tmp_coord, " !\"#$%&'()*+,-./0123456789" );
        tmp_coord += 1;
        paint_text_col_row( 0, tmp_coord, ":;<=>?@[\\]^_`{|}~" );
        tmp_coord += 1;
        paint_text_col_row( 0, tmp_coord, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
        tmp_coord += 1;
        paint_text_col_row( 0, tmp_coord, "abcdefghijklmnopqrstuvwxyz" );

    CASE_TEST_NEXT:
        paint_screen_clear();
        tmp_coord = 0;
        xsprintf( str, "абвгд" );
        str_UTF_to_ANSI( str );
        paint_text_col_row( 0, tmp_coord, str );
        tmp_coord += 1;
        xsprintf( str, "АБВГДЕЁЖЗИКЛМНОПРСТУФХЧЩЪЬЭЮЯ" );
        str_UTF_to_ANSI( str );
        paint_text_col_row( 0, tmp_coord, str );

    CASE_TEST_NEXT:
        paint_screen_clear();
        //Fract();
        //Draw();

    default: test_num = 0; break;
    }

    hal_paint_screen_update();
}

#ifdef STM32
#include "hal.h"
#endif

void hal_screen_startup_out( void )
{
    char str[ 32 ];

    paint_init( SCREEN_ORIENTATION_NORMAL );
    paint_font( PAINT_FONT_Generic8, PAINT_FONT_MS );
    paint_color_set( COLOR_WHITE );
    paint_color_background_set( COLOR_BLACK );
    paint_screen_clear();

#ifdef BOARD_DESCRIPTOR
    xsprintf( str, "DD:%s", BOARD_DESCRIPTOR );
    paint_msg_println( str );
    paint_screen_update();
#endif

    xsprintf( str, "FWD:%08u", (U32)BUILD_DATE );
    paint_msg_println( str );
    xsprintf( str, "FWT:%06u", (U32)BUILD_TIME );
    paint_msg_println( str );

#ifdef STM32
    xsprintf( str, "UID0:%08X\n", halMCU_UID_get( 0 )); //example: for #1-003C0036, 003C002B
    paint_msg_println( str );
    xsprintf( str, "UID1:%08X\n", halMCU_UID_get( 1 )); //example: for #1-34355118, 3331470E
    paint_msg_println( str );
    xsprintf( str, "UID2:%08X\n", halMCU_UID_get( 2 )); //example: for #1-36363736, 35373532
    paint_msg_println( str );
#endif

    paint_screen_update();
}

