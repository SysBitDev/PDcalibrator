#include "modParser.h"
#include "board.h"
#if MODPARCER_CRC_CHECK
#include "_crc.h"
#endif
#include "_misc.h"
#include <string.h>


enum PARSER_STATE {
    PARSER_STATE_RESET = 0,
    PARSER_STATE_IDLE,
    PARSER_STATE_CH_ADR,
    PARSER_STATE_CH_LEN_L,
    PARSER_STATE_CH_LEN_H,
    PARSER_STATE_REC_DATA,
    PARSER_STATE_CRC_L,
    PARSER_STATE_CRC_H,
    PARSER_STATE_CH_END
};

RET CODE modParser_init( parser_t *p, U8 addr, U32 max_size  )
{
    test_param( NULL == p );
    test_param( max_size > PARSER_MAX_DATA_SIZE );

    p->addr = addr;
    p->state = PARSER_STATE_IDLE;
    p->recive_cnt_max = max_size;

    return RET_OK;
}

RET CODE modParser_decode( parser_t *p, U8 rchar, U8 *buf_out, INT *size_buf_out )
{
    RET respond = RET_BUSY;

    test_param( NULL == p );
    test_param( NULL == buf_out );
    test_param( NULL == size_buf_out );

    switch( p->state )
    {
    case PARSER_STATE_IDLE: // START байт
        if( PIK_START == rchar ) // wait start preamble
        {
            p->state = PARSER_STATE_CH_ADR;
        }
        break;

    case PARSER_STATE_CH_ADR:
        if( p->addr == rchar ) // dev address
        {
            p->state = PARSER_STATE_CH_LEN_L;
        }
        else
        {
            if( PIK_START != rchar ) // if still preamble, wait
            {
                p->state = PARSER_STATE_IDLE; // error
            }
        }
        break;

    case PARSER_STATE_CH_LEN_L:
        p->reciv_cnt = 0;
        p->size_buf_size = rchar;
        p->state = PARSER_STATE_CH_LEN_H;
        break;

    case PARSER_STATE_CH_LEN_H:
        p->size_buf_size |= (U16)( rchar << 8 );
        // sanity check
        if(( p->size_buf_size == 0 ) || ( p->size_buf_size > p->recive_cnt_max ) )
        {
            p->crc = 0;
            respond = RET_ERROR;
            p->state = PARSER_STATE_IDLE;
        }
        else
        {
            p->crc_rx = 0xFFFF; // for 16 bit CRC
            p->state = PARSER_STATE_REC_DATA;
        }
        break;

    case PARSER_STATE_REC_DATA: // reciving data
        buf_out[ p->reciv_cnt ] = rchar; // fill out buffer
        // compute CRC
#if MODPARCER_CRC_CHECK
        p->crc_rx = CRC16_MODBUS( &rchar, 1, p->crc_rx );
#endif
        p->reciv_cnt++;
        if( p->reciv_cnt >= p->size_buf_size )
        {
            p->crc = 0;
            p->state = PARSER_STATE_CRC_L;
        }
        break;

   case PARSER_STATE_CRC_L: // crc
        p->crc = (U16)rchar;
        p->state = PARSER_STATE_CRC_H;
        break;

   case PARSER_STATE_CRC_H: // check crc
        p->crc |= (U16)(rchar << 8);
#if MODPARCER_CRC_CHECK
        if( p->crc_rx == p->crc )
#endif
        {
            p->state = PARSER_STATE_CH_END;
        }
#if MODPARCER_CRC_CHECK
        else
        {
            respond = RET_ERROR;
            p->state = PARSER_STATE_IDLE;
        }
#endif
        break;

    case PARSER_STATE_CH_END:
        if( PIK_FIN == rchar )
        {
        	*size_buf_out = p->size_buf_size;
            respond = RET_OK; // recive, all succesfull
        }
        p->state = PARSER_STATE_IDLE; // to start otherwise
        break;

    default:
        respond = RET_ERROR;
        p->state = PARSER_STATE_IDLE;
        break;
    }

    return respond;
}

RET CODE modParser_encode( parser_t *p, U8 *buf_in, INT buf_in_size, U8 *buf_out, INT *size_buf_out )
{
    test_param( NULL == p );
    test_param( NULL == buf_in );
    test_param( 0 == buf_in_size );
    test_param( buf_in_size > PARSER_MAX_DATA_SIZE );
    test_param( NULL == buf_out );
    test_param( NULL == size_buf_out );

    buf_out[ 0 ] = PIK_START;

    buf_out[ 1 ] = p->addr;

    buf_out[ 2 ] = (U8)( buf_in_size & 0x00FF ); // LEN
    buf_out[ 3 ] = (U8)( buf_in_size >> 8 ); // LEN

    if( &buf_out[ PARSER_STRUCT_HEAD_SIZE ] != buf_in ) //zero copy test, write to this!
    {
        //aligned
        _memcpy( &buf_out[ PARSER_STRUCT_HEAD_SIZE ], buf_in, buf_in_size );
    }

    p->crc_rx = 0xFFFF; // for this CRC

#if MODPARCER_CRC_CHECK
    p->crc_rx = CRC16_MODBUS( buf_in, buf_in_size, p->crc_rx );
#endif

    buf_out[ 4 + buf_in_size ] = (U8)( p->crc_rx & 0x00FF ); // CRC
    buf_out[ 5 + buf_in_size ] = (U8)( p->crc_rx >> 8 );

    buf_out[ 6 + buf_in_size ] = PIK_FIN; //finalize packet

    *size_buf_out = PARSER_STRUCT_HEAD_SIZE + buf_in_size + PARSER_STRUCT_TAIL_SIZE;

    return RET_OK;
}

RET CODE modParser_test( VOID )
{
	parser_t parser;
	U8 parser_buf[ PARSER_MAX_DATA_SIZE ];
	U8 parser_buf_out[ 4 ];
	INT size;

	// parser init =============================================================
  	modParser_init( &parser, PIK_ADR_DEFAULT, PARSER_MAX_DATA_SIZE );

    while( 1 )
    {
    	U8 char_tmp = 4;
        //if( RET_OK == halUSART1_rcvS (&char_tmp8) )
        {
            if( RET_OK != modParser_decode( &parser, char_tmp, &parser_buf[0], &size ))
            {
            	test_param(1);
            }
        	if( RET_OK != modParser_encode( &parser, &parser_buf[0], size, &parser_buf_out[0], &size ) )
        	{
        		test_param(1);
        	}
        	break;
        }
    }

    return RET_OK;
}
