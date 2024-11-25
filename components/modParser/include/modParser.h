/**
 * @file    modParser.h
 * @author  Антон Логинов, Ht3h5793, CD45
 * @date    13.6.2013  10:10
 * @version V1.0.2
 * @brief   

// Структура пакета - xxxxx::,P,0x0064=len,data[len],0x5821=crc,;,xxxxxx
// где ::: - преамбула, для очистки парсера

*/

#ifndef MODPARSER_H
#define	MODPARSER_H 20191031

#include "board.h"


#ifndef PIK_START
    #define PIK_START                   0xFF //'\r' //0x0D
#endif

#ifndef PIK_ADR_DEFAULT
	#define PIK_ADR_DEFAULT          	((U8)'#')
#endif

#ifndef PARSER_MIN_DATA_SIZE
    #define PARSER_MIN_DATA_SIZE        (1)
#endif

#ifndef PARSER_MAX_DATA_SIZE
    #define PARSER_MAX_DATA_SIZE        (4096)
#endif

#ifndef PIK_FIN
    #define PIK_FIN                     0xFF //'\n' //0x0A
#endif

#define PARSER_STRUCT_HEAD_SIZE         4 //1 byte START + 1 byte adress + 2 bytes size
#define PARSER_STRUCT_TAIL_SIZE         3 //DATAGRAM_SIZE 2 byte CRC + 1 byte FIN
#define PARSER_STRUCT_ALL_SIZE          ( PARSER_STRUCT_HEAD_SIZE + PARSER_STRUCT_TAIL_SIZE )

#define PARSER_MAX_PACKET_SIZE          ( PARSER_STRUCT_HEAD_SIZE + PARSER_MAX_DATA_SIZE + PARSER_STRUCT_TAIL_SIZE )


typedef struct parser_t_ {
    U8  state; 		// позиция внутреннего автомата, не трогать
    U8  addr; 		// адрес, на который нужно ответить
    INT reciv_cnt;
    INT recive_cnt_max;
    U16 size_buf_size;
    U16 crc_rx; 	// CRC reciving
    U16 crc; 		// CRC computing
    U8 *pBufOUT;
} parser_t;


//#pragma pack(push,1)
//PACK_START
typedef struct parcer_command_t_ {
    U8  command;
    U32 address;
    U16 size;

} parcer_command_t;
//PACK_END;
//#pragma pack(pop)


#ifdef	__cplusplus
extern "C" {
#endif


/**
 * @brief modParser_init
 * @param p - pointer parser struct
 * @param adress
 * @param max_size
 * @return
 */
RET CODE modParser_init( parser_t *p, U8 adress, U32 max_size ); // PIK_ADR


/**
 * @brief modParser_decode
 * @param p - pointer parser struct
 * @param buf_in
 * @param buf_out
 * @param size_buf_out
 * @return
 */
RET modParser_decode( parser_t *p, U8 rchar, U8 *buf_out, INT *size_buf_out );


/**
 * @brief modParser_encode
 * @param p - pointer parser struct
 * @param buf_in
 * @param buf_in_size
 * @param buf_out
 * @param size_buf_out
 * @return
 */
RET modParser_encode( parser_t *p, U8 *buf_in, INT buf_in_size, U8 *buf_out, INT *size_buf_out );


#ifdef	__cplusplus
}
#endif

#endif	/* MODPARSER_H */
