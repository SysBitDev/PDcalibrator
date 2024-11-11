#include "colors.h"
#include "board.h"


//#ifndef COLOR_BI

U8 CODE color_get_R8( COLOR color )
{
    return (COLOR)((color >> COLOR_R_OFFSET) << COLOR_R_OFFSET_MASK);
}


U8 CODE color_get_G8( COLOR color )
{
    return (COLOR)((color >> COLOR_G_OFFSET) << COLOR_G_OFFSET_MASK);
}


U8 CODE color_get_B8( COLOR color )
{
    return (COLOR)((color >> COLOR_B_OFFSET) << COLOR_B_OFFSET_MASK);
}


U8 CODE color_RGB888_to_RGB8( U8 r, U8 g, U8 b )
{
    return (U8)((((r & 0xE0) >> 5) << 5) | (((g & 0xE0) >> 5) << 2) | ((b & 0xC0) >> 6));
}


U16 CODE color_RGB888_to_RGB565( U32 c )
{
    return (U16)( ((c & 0x00F80000) >> 8 ) | ((c & 0x0000FC00) >> 5) | ((c & 0x000000F8) >> 3) );
}


COLOR CODE color_RGB888_to_RGB565_3v( U8 r, U8 g, U8 b )
{
    return (U16)((r & 0xF8) << 8) | (U16)((g & 0xFC) << 3 ) | (U16)(b >> 3);
}


void CODE color_RGB565_to_RGB888( COLOR color, U8 *pColor )
{
    pColor[0] = ((color & 0x001F) << 3) | (color & 0x07);          /* Blue value */
    pColor[1] = ((color & 0x07E0) >> 3) | ((color >> 7) & 0x03);   /* Green value */
    pColor[2] = ((color & 0xF800) >> 8) | ((color >> 11) & 0x07);  /* Red value */
}




//(COLORS_8BIT || COLORS_12BIT || COLORS_16BIT || COLORS_18BIT || COLORS_24BIT || COLORS_32BIT)
COLOR CODE paint_blend_2_colors (COLOR fg, COLOR bg, U8 alpha)
{
    U8 colorBuf[3];
    U16 r, g, b;
    U16 fg_ratio = alpha + 1;
    U16 bg_ratio;

    if (alpha == 0)
    {
        return bg;
    }
    if (alpha < 255)
    {
        color_RGB565_to_RGB888( fg, &colorBuf[0] );
        bg_ratio = 256 - alpha;
        r = color_get_R8(fg) * fg_ratio;
        g = color_get_G8(fg) * fg_ratio;
        b = color_get_B8(fg) * fg_ratio;
        r += color_get_R8(bg) * bg_ratio;
        g += color_get_G8(bg) * bg_ratio;
        b += color_get_B8(bg) * bg_ratio;
        r /= 256;
        g /= 256;
        b /= 256;
        return color_RGB888_to_RGB565_3v( r, g, b );
    }
    else
    {
        return fg;
    }
}


COLOR color_get_half( COLOR color )
{
    U32 cr = ( color & ( COLOR_R_MASK << COLOR_R_OFFSET)) >> 1;
    cr = cr & ( COLOR_R_MASK << COLOR_R_OFFSET);
    U32 cg = ( color & ( COLOR_G_MASK << COLOR_G_OFFSET)) >> 1;
    cg = cg & ( COLOR_G_MASK << COLOR_G_OFFSET);
    U32 cb = ( color & ( COLOR_B_MASK << COLOR_B_OFFSET)) >> 1;
    cb = cb & ( COLOR_B_MASK << COLOR_B_OFFSET);
    return cr | cg | cb;
}


const U8 color_gamma_correction_8b[ 256 ] = {
  0, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6,
  6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8,
  8, 8, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11,
  11, 11, 12, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15,
  15, 15, 16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20,
  20, 20, 21, 21, 22, 22, 22, 23, 23, 24, 24, 25, 25, 25, 26, 26,
  27, 27, 28, 28, 29, 29, 30, 30, 31, 32, 32, 33, 33, 34, 35, 35,
  36, 36, 37, 38, 38, 39, 40, 40, 41, 42, 43, 43, 44, 45, 46, 47,
  48, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
  63, 64, 65, 66, 68, 69, 70, 71, 73, 74, 75, 76, 78, 79, 81, 82,
  83, 85, 86, 88, 90, 91, 93, 94, 96, 98, 99, 101, 103, 105, 107, 109,
  110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
  146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
  193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,
};

/*------------------------------------------------------------------------------
  Конвертер из HSV в RGB в целочисленной арифметике

  hue        : 0..360
  saturation : 0..255
  value      : 0..255
 ------------------------------------------------------------------------------*/
COLOR HSV_to_RGB( INT hue, INT sat, INT val )
{
    INT r;
    INT g;
    INT b;
    INT base;
    U32 rgb;

    //sat = 255 - color_gamma_correction_8b[ 255 - sat ];

    if ( sat == 0 ) // Acromatic color (gray). Hue doesn't mind.
    {
        rgb = val | ( val << 8 ) | ( val << 16 );
    }
    else
    {
        base = ((255 - sat) * val) >> 8;
        switch( hue / 60 )
        {
        case 0:
            r = val;
            g = (((val - base) * hue) / 60) + base;
            b = base;
            break;

        case 1:
            r = (((val - base) * (60 - (hue % 60))) / 60) + base;
            g = val;
            b = base;
            break;

        case 2:
            r = base;
            g = val;
            b = (((val - base) * (hue % 60)) / 60) + base;
            break;

        case 3:
            r = base;
            g = (((val - base) * (60 - (hue % 60))) / 60) + base;
            b = val;
            break;

        case 4:
            r = (((val - base) * (hue % 60)) / 60) + base;
            g = base;
            b = val;
            break;

        case 5:
            r = val;
            g = base;
            b = (((val - base) * (60 - (hue % 60))) / 60) + base;
            break;
        default: r = g = b = 0; break;
        }

//        if( COLOR_BLUE == 0x000000FF )//WTF???
//            rgb = U32(((r & 0xFF)<<16) | ((g & 0xFF)<<8) | (b & 0xFF));
//        else
            rgb = (r<<16) | (g<<8) | b;//color_RGB_repack( r, g, b );
    }
#if COLOR_RGB565
    rgb = _color_RGB888_to_RGB16( rgb );
#endif

    return rgb;
}

void HSV_to_RGB888( U16 hue, U8 *r, U8 *g, U8 *b )
{
    //sat = 255 - dim_curve[255 - sat];
    // sector specifies which colors are primary and secondary
    U8 sector = hue >> 8;
    // calculate secondary color value from hue
    U8 secondary =  hue & 0xFF; //(U32)hue % 0xFF - ((U32)sector % 2) * 0xFF ;

    switch( sector )
    {
    case 0:
        *r = 0xFF;
        *g = secondary;
        *b = 0;
        break;

    case 1:
        *r = 0xFF - secondary;
        *g = 0xFF;
        *b = 0;
        break;

    case 2:
        *r = 0;
        *g = 0xFF;
        *b = secondary;
        break;

    case 3:
        *r = 0;
        *g = 0xFF - secondary;
        *b = 0xFF;
        break;

    case 4:
        *r = secondary;
        *g = 0;
        *b = 0xFF;
        break;

    case 5:
        *r = 0xFF;
        *g = 0;
        *b = 0xFF - secondary;
        break;

    default:
        *r = 0;
        *g = 0;
        *b = 0;
        break;
    }
}

void color_make_rainbow_palette( COLOR *pbuf, INT size )
{
    U32 hue;
    for( INT i = 0; i < size; i++ )
    {
        hue = (( 250 * i ) / size );
        pbuf[ i ] = HSV_to_RGB( hue, 255, 255 );
    }
}

void color_test( void )
{
    U32 color;

    color = 0x000000FF;
    color = color_RGB888_to_RGB565( color );
    //xsprintf( str, "%08X\r\n", color );
    //sys_print( str );

    color = 0x0000FF00;
    color = color_RGB888_to_RGB565( color );
    //xsprintf( str, "%08X\r\n", color );
    //sys_print( str );

    color = 0x00FF0000;
    color = color_RGB888_to_RGB565( color );
    //xsprintf( str, "%08X\r\n", color );
    //sys_print( str );
//    AMG8833_temp[0] = color_RGB888_to_RGB565( 0x00FF0000 );
//    AMG8833_temp[0] = color_RGB888_to_RGB565( 0x0000FF00 );
//    AMG8833_temp[0] = color_RGB888_to_RGB565( 0x000000FF ); TODO

}
