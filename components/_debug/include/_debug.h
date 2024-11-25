/* 
 * @file    _debug.h
 * @author  Ht3h5793
 * @date    08.09.2013
 * @version V3.5.1
 * @brief   
 *
for debug:
...
	U32 tic = 0;
	_debug_init(); //on MCU debug
	RESET_CORE_COUNT;
	... our test code.. (WARNING! no _delay there, its not ok!!!)
	tic = GET_CORE_COUNT;

 */

#ifndef _DEBUG_H
#define	_DEBUG_H 20230219

#include "board.h" 


#ifdef STM32
#define DWT_CYCCNT                      *(volatile U32 *)0xE0001004
#define DWT_CONTROL                     *(volatile U32 *)0xE0001000
#define SCB_DEMCR                       *(volatile U32 *)0xE000EDFC
#define DBGMCU_CR                       *(volatile U32 *)0xE0042004 // debug on

#ifndef RESET_CORE_COUNT
#define RESET_CORE_COUNT                (DWT_CYCCNT = 0)
#endif

#ifndef GET_CORE_COUNT
#define GET_CORE_COUNT                  (DWT_CYCCNT)
#endif

#endif //STM32

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief Init core counter tick (only for >M1, M3, M4 end high )
 * @retval - none
 */
void _debug_init( void );


/**
* @brief
* @retval 1 - debug mode enable
*         0 - debug mode disable
*/
BOOL debug_debug_mode_get( void );


#if __DEBUG


/**
 *
 */
void debug_printf( const char *fmt, ... );


/**
 *
 */
void debug_put( const char *p );


#else

#define debug_printf(...)

#endif


#ifdef  __cplusplus
}
#endif

#endif /** _DEBUG_H */
