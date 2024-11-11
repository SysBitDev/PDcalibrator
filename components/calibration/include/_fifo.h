/**
 * @file    _fifo.h
 * @author  Ht3h5793, CD45
 * @date    13.12.2012  15:50
 * @version V100.30.2
 * @brief   
 * @todo 
 * 

        DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2022, December 2022

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.

 */

#ifndef FIFO_H
#define	FIFO_H 20191213

/** include */
#include "board.h"


/** define */


/** typedef */
typedef struct FIFO_ {//struct
    S32 in; // Next In Index
    S32 out; // Next Out Index
    U8 *buf; // Buffer
    S32 buf_size; // Size buffer
	INT item_size; // Size in bytes for each element in the buffer.
} FIFO;


#ifdef	__cplusplus
extern "C" {
#endif

/**
 * @brief fifo_init_static - Init a statically allocated FIFO buffer
 * @param fifo - The pointer to the structure holding FIFO data
 * @param pbuf - Pointer to the memory used to store actual fifo items
 * @param fifo_size - The number of items to store on the FIFO
 * @param item_size - The size in bytes of each item on the buffer
 * @return - @p TRUE on success, @p FALSE otherwise
 */
BOOL _fifo_init_static( FIFO *fifo, VOID *pbuf, U32 fifo_size, U32 item_size );


/**
 * @brief fifo_flush
 * @param fifo
 * @return  - TRUE or FALSE
 */
BOOL _fifo_flush( FIFO *fifo );


/**
 * @brief fifo_available
 * @param fifo
 * @return  - TRUE or FALSE
 */
S32 _fifo_available( FIFO *fifo );


/**
 * @brief fifo_free
 * @param fifo
 * @return  - TRUE or FALSE
 */
S32 _fifo_free( FIFO *fifo );


/**
 * @brief fifo_add
 * @param fifo
 * @param item
 * @return  - TRUE or FALSE
 */
BOOL _fifo_add( FIFO *fifo, const VOID *item );


/**
 * @brief fifo_get
 * @param fifo
 * @param item
 * @return  - TRUE or FALSE
 */
BOOL _fifo_get( FIFO *fifo, VOID *item );


/**
 * @brief fifo_adds
 * @param fifo
 * @param item
 * @param num
 * @return  - TRUE or FALSE
 */
BOOL _fifo_adds( FIFO *fifo, const void *item, INT num );


/**
 * @brief fifo_gets
 * @param fifo
 * @param item
 * @param num
 * @return  - TRUE or FALSE
 */
BOOL _fifo_gets( FIFO *fifo, VOID *item, INT num );


/**
 * @brief test
 */
void _fifo_test( VOID );


#ifdef	__cplusplus
}
#endif

#endif	/* FIFO_H */
