#include "modGUI.h"
#include "modPaint.h"
#include "board.h"
//#include "modSysClock.h"
#include "_misc.h"


void modGUI_button_draw( button_t *b, INT state, char *text )
{
    paint_color_set( state ? b->color_pressed : b->color );
    paint_rectangle_round( b->x, b->y, b->w, b->h, 4, FALSE );
//    paint_color_set( COLOR_ORANGE );
//    paint_rectangle_round( b->x+1, b->y+1, b->w-1, b->h-2, 4, TRUE );
    paint_color_set( COLOR_ORANGE );
    paint_color_background_set( COLOR_BLACK );
    COORD x = b->w / 2 - ( _strlen( text ) * paint_font_w_get() ) / 2;
    if( x < 0 ) x = 0;
    COORD y = b->h / 2 - paint_font_h_get() / 2 + 1;
    paint_text_xy( b->x + x, b->y + y, text );
    paint_color_background_set( COLOR_BLACK ); // return color
}

RET modGUI_init( void )
{
    return RET_OK;
}

RET    modGUI_run(uint8_t key)
{
//     guiPaint.setBackgroundColor(COLOR_BLUE);
//     guiPaint.clearScreen();
//     guiPaint.setColor(COLOR_WHITE);
//     uint32_t i;
//     for (i = 1; i < (TEXT_MAX_COL - 1); i++)
//     {
//         guiPaint.putCharXY( i, 0, 128);
//     }
//     for (i = 1; i < (TEXT_MAX_COL - 1); i++)
//     {
//         guiPaint.putCharXY( i, TEXT_MAX_ROW - 1, 128);
//     }
//     for (i = 1; i < (TEXT_MAX_ROW - 1); i++)
//     {
//         guiPaint.putCharXY( 0, i, 129);
//     }
//     for (i = 1; i < (TEXT_MAX_ROW - 1); i++)
//     {
//         guiPaint.putCharXY( TEXT_MAX_COL - 1, i, 129);
//     }
//     guiPaint.putCharXY( 0, 0, 130);
//     guiPaint.putCharXY( 0, TEXT_MAX_ROW - 1, 131);
//     guiPaint.putCharXY( TEXT_MAX_COL - 1, 0, 132);
//     guiPaint.putCharXY( TEXT_MAX_COL - 1, TEXT_MAX_ROW - 1, 133);

//     // рисуем титл
//     const char strMain[] = {"BOOT"};
//     guiPaint.setColor(0x4333);
//     uint16_t size = sizeof(strMain);
//     guiPaint.putStringXY( ((TEXT_MAX_COL - 1) / 2) - (size / 2, 0), 0, (char *)&strMain[0]);
//     
//     // пишем элементы меню
//     guiPaint.setColor(COLOR_GREEN);
//     guiPaint.putStringXY( 1, 1, (char *)"ERROR!");
//     guiPaint.putStringXY( 1, 2, (char *)"ќЎ»Ѕ ј!");
//     
//     guiPaint.putCharXY(1, 5, key);
//     
//     // мигаем курсором :)
//     if (modSysClock_getPastTime(CURSOR.timer, SYSCLOCK_GET_TIME_MS_1) > 500)
//     {
//         CURSOR.timer = modSysClock_getTime();
//         if (1 == CURSOR.cursor)
//         {
//             CURSOR.cursor = 0;
//             guiPaint.putCharXY( CURSOR.x, CURSOR.y, ' ');
//         }
//         else
//         {
//             CURSOR.cursor = 1;
//             guiPaint.putCharXY( CURSOR.x, CURSOR.y, '-');
//         }
//     }
    return RET_OK;
}
