/*
 * @file    GUI
 * @author  Ht3h5793
 * @date    04.04.2014
 * @version V0.0.1
 * @brief  
 *
*/

#ifndef MODGUI_H
#define	MODGUI_H 20240407 /* Revision ID */

#include "colors.h"
#include "board.h"
#include "common.h"
#include "modPaint.h"


typedef struct button_t_ {
    COORD x; COORD y; COORD w; COORD h;
    FAST_U8 state;
    COLOR color;
    COLOR color_pressed;
} button_t;


#ifdef	__cplusplus
extern "C" {
#endif

/**
 * @brief modGUI_button_draw
 * @param pbutton
 * @param state
 * @param text
 */
void modGUI_button_draw( button_t *pbutton, INT state, char *text );

#ifdef	__cplusplus
}
#endif

#endif	/* MODGUI_H */
