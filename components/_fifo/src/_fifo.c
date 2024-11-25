#include "_fifo.h"
#include "board.h"
#include "_misc.h"


#define FIFO_2X             1 // fast mode

//#define FIFO_ALIGN(value, alignment) ((value + alignment - 1) & ~(alignment - 1))
//#define ADVANCE_QUEUE(index, size, max_size) ((index + size) % max_size) 

BOOL _fifo_init_static( FIFO *fifo, void *pbuf, U32 fifo_size, U32 item_size )
{
	//Sanity check
	test_param( NULL == fifo );
	test_param( NULL == pbuf );
	test_param(fifo_size < 2 ); // Size is too small.  It must be larger than 2
#if FIFO_2X
	test_param((fifo_size & (fifo_size - 1)) != 0 ); // Must be a power of 2
#endif
	test_param( item_size > fifo_size );
    {
		//return FALSE;
    }
    fifo->buf = (U8 *)pbuf;
	fifo->buf_size = fifo_size;
	fifo->item_size = item_size;
    _fifo_flush( fifo );
	return TRUE;
}

BOOL _fifo_flush( FIFO *fifo )
{
	test_param( NULL == fifo );
	fifo->in = 0;
	fifo->out = fifo->in;
	return TRUE;
}

S32 _fifo_available( FIFO *fifo )
{
	test_param( NULL == fifo );
    S32 i = ( fifo->in - fifo->out );
    if( i < 0 )
    {
    	i = -i;
    }
    return i;
}

S32 _fifo_free( FIFO *fifo )
{
	test_param( NULL == fifo );
	return ( fifo->buf_size - _fifo_available( fifo ) );
}

BOOL _fifo_add( FIFO *fifo, const void *item )
{
	U32 offset;
    
	test_param( NULL == fifo );
	test_param( NULL == item );
	if( _fifo_free( fifo ) >= fifo->item_size )
	{
		offset = fifo->in & ( fifo->buf_size - 1 );
        fifo->in += fifo->item_size;
		_memcpy( fifo->buf + offset, 
                item, 
                fifo->item_size );
		return TRUE;
	}
	return FALSE;
}

BOOL _fifo_get( FIFO *fifo, void *item )
{
	U32 offset;

	test_param( NULL == fifo );
	test_param( NULL == item );

	if( _fifo_available( fifo ) >= fifo->item_size )
	{
		offset = fifo->out & ( fifo->buf_size - 1 );
        fifo->out += fifo->item_size;
		_memcpy( item, fifo->buf + offset, fifo->item_size );
		return TRUE;
	}
    return FALSE;
}

BOOL _fifo_adds( FIFO *fifo, const void *item, INT num )
{
	register INT j, offset;
    register U8 *p = (U8 *)item;
	BOOL ret = FALSE;

	test_param( NULL == fifo );
	test_param( NULL == item );
	test_param( num < 0 );

	if( _fifo_free( fifo ) >= ( fifo->item_size * num ) )
	{
		for( j = 0; j < num; j++ )
		{
			offset = fifo->in & ( fifo->buf_size - 1 );
            fifo->in += fifo->item_size;
            
			_memcpy( fifo->buf + offset,
                (U8 *)( p + j * fifo->item_size ),
                fifo->item_size );
		}
		ret = TRUE;
	}

	return ret;
}

BOOL _fifo_gets( FIFO *fifo, void *item, INT num )
{
	register INT j, offset;
    U8 *p = (U8 *)item;
	BOOL ret = FALSE;

	test_param( NULL == fifo );
    test_param( NULL == item );
	test_param( num < 1 );
    
	if( _fifo_available( fifo ) >= ( fifo->item_size * num ) )
	{
        for( j = 0; j < num; j++ )
		{
            offset = fifo->out & ( fifo->buf_size - 1 );
            fifo->out += fifo->item_size;
            
            _memcpy( p + j * fifo->item_size,
                fifo->buf + offset,
                fifo->item_size );
        }
		ret = TRUE;
	}

    return ret;
}

void _fifo_test( void )
{
    U8 test_buf[ 64 ];
    FIFO fifo;
    U8 fifo_buf[ 32 ];
    INT i;
    
    /*
    if( TRUE ==  _fifo_init_static( &NULL, fifo_buf, 32, 1 ) )
    {
        while( 1 ){};
    }
    if( TRUE ==  _fifo_init_static( &fifo, NULL, 32, 1 ) )
    {
        while( 1 ){};
    }
    */
    
    if( TRUE !=  _fifo_init_static( &fifo, fifo_buf, 32, 1 ) )
    {
        while( 1 ){};
    }
    
    test_buf[ 0 ] = 0; //clear before
    if( TRUE == _fifo_get( &fifo, &test_buf[ 0 ] ) )
    {
        while( 1 ){}; 
    }
    
    test_buf[ 0 ] = 8;
    if( TRUE != _fifo_add( &fifo, &test_buf[ 0 ] ) )
    {
        while( 1 ){}; 
    }
    
    test_buf[ 0 ] = 0; //clear before
    if( TRUE != _fifo_get( &fifo, &test_buf[ 0 ] ) )
    {
        while( 1 ){}; 
    }
    
    for( i = 0; i < 5; i++ ) //init before
    {
        test_buf[ i ] = i + 1;
    }
    if( TRUE != _fifo_adds( &fifo, &test_buf[ 0 ], 5 ) )
    {
        while( 1 ){}; 
    }
    
    i = _fifo_available( &fifo );
    
    for( i = 0; i < 5; i++ ) //clear before
    {
        test_buf[ i ] = 0;
    }
    if( TRUE == _fifo_gets( &fifo, &test_buf[ 0 ], 8 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_gets( &fifo, &test_buf[ 0 ], 5 ) )
    {
        while( 1 ){}; 
    }
    else
    {
        for( i = 0; i < 5; i++ )
        {
            if( test_buf[ i ] != ( i + 1 ) )
            {
                while( 1 ){}; 
            }
        }
    }
    
    //max limit test
    if( TRUE == _fifo_adds( &fifo, &test_buf[ 0 ], 100 ) )
    {
        while( 1 ){}; 
    }
    
    // overflov test
    for( i = 0; i < 32; i++ ) //init before
    {
        test_buf[ i ] = i + 1;
    }
    if( TRUE != _fifo_adds( &fifo, &test_buf[ 0 ], 31 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_gets( &fifo, &test_buf[ 0 ], 30 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_adds( &fifo, &test_buf[ 0 ], 31 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_gets( &fifo, &test_buf[ 0 ], 30 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_adds( &fifo, &test_buf[ 0 ], 30 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_gets( &fifo, &test_buf[ 0 ], 30 ) )
    {
        while( 1 ){}; 
    } 
    
    // 2byte
    if( TRUE !=  _fifo_init_static( &fifo, fifo_buf, 32, 2 ) )
    {
        while( 1 ){};
    }
    for( i = 0; i < ( 32 / 2 ); i += 2 ) //init before
    {
        test_buf[ i + 0 ] = i / 2 + 1;
        test_buf[ i + 1 ] = 0;
    }
    if( TRUE != _fifo_adds( &fifo, &test_buf[ 0 ], 7 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_gets( &fifo, &test_buf[ 0 ], 6 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_adds( &fifo, &test_buf[ 0 ], 7 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_gets( &fifo, &test_buf[ 0 ], 6 ) )
    {
        while( 1 ){}; 
    } 
    if( TRUE != _fifo_adds( &fifo, &test_buf[ 0 ], 7 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_gets( &fifo, &test_buf[ 0 ], 7 ) )
    {
        while( 1 ){}; 
    } 
    
    // 3byte
    if( TRUE !=  _fifo_init_static( &fifo, fifo_buf, 32, 3 ) )
    {
        while( 1 ){};
    }
    for( i = 0; i < ( 32 / 3 ); i += 3 ) //init before
    {
        test_buf[ i + 0 ] = i / 3 + 1;
        test_buf[ i + 1 ] = 0;
        test_buf[ i + 2 ] = 0;
    }
    if( TRUE != _fifo_adds( &fifo, &test_buf[ 0 ], 4 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_gets( &fifo, &test_buf[ 0 ], 3 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_adds( &fifo, &test_buf[ 0 ], 4 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_gets( &fifo, &test_buf[ 0 ], 3 ) )
    {
        while( 1 ){}; 
    } 
    if( TRUE != _fifo_adds( &fifo, &test_buf[ 0 ], 4 ) )
    {
        while( 1 ){}; 
    }
    if( TRUE != _fifo_gets( &fifo, &test_buf[ 0 ], 3 ) )
    {
        while( 1 ){}; 
    } 
}
