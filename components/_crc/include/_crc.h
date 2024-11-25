/**
 * @file    _crc.h
 * @author  HT3H5796-12, Ht3h5793, CD45
 * @date    13.6.2013  15:50
 * @version V1.0.2
 * @brief   varios software CRC
 * @todo


    U8 buf[] = { "123456789" };
    U8 size = 9; //sizeof(buf);

    // Dallas CRC
    U8 crc_8DS;
    crc_8DS = crc8_DS (&buf[0], size);
    if (0xA1 != crc_8DS) while(1){};
    crc_8DS = 0x00;
    for( U32 i = 0; i < size; i++)
    {
        crc8_DS_s (&crc_8DS, &buf[i]);
    }
    if (0xA1 != crc_8DS) while(1){};

    // CRC 16 CITT
    U16 crc_16;
    crc_16 = crc16_CITT (&buf[0], size);
    if (0x29B1 != crc_16) while(1){};
    crc_16 = 0xFFFF;
    for( U32 i = 0; i < size; i++)
    {
        crc16_CITT_s (&crc_16, &buf[i]);
    }
    if (0x29B1 != crc_16) while(1){};


    // CRC 32
    U32 crc_32;
    crc_32 = crc32_CCITT (&buf[0], size);
    if (0x89A1897F != crc_32) while(1){};
    crc_32 = 0x00000000;//FFFFFFFF;
    for( U32 i = 0; i < size; i++)
    {
        crc32_CCITT_s (&crc_32, &buf[i]);
    }
    if (0x89A1897F != crc_32) while(1){};

    



http://reveng.sourceforge.net/crc-catalogue/all.htm
https://en.wikipedia.org/wiki/Cyclic_redundancy_check
http://www.zorc.breitbandkatze.de/crc.html
http://ghsi.de/CRC/
http://www.sunshine2k.de/coding/javascript/crc/crc_js.html

 */

#ifndef _CRC_H
#define	_CRC_H 20160611

/** include */

#include "board.h"


/** typedef */

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * @brief CRC1 - Parity of byte
 * @param data
 * @return - 0 or 1
 */
INT CRC1( U8 data );


/**
 * @brief 8 bit CRC for common use
    Name  : CRC-8 Dallas/Maxim
    Poly  : 0x31    x^8 + x^5 + x^4 + 1 (0x31 / 0x8C / 0x98)
    Init  : 0x00
    Revert: false
    XorOut: 0x00
    Check : 0xA1 ("123456789") size = 9
    MaxLen:
 * @param src
 * @param len
 * @param crc
 * @return
 */
U8 CRC8_DS( void *src, U8 len, U8 crc );


/**
 * @brief CRC16_MODBUS
    MAXIM  0x0000, 0xFFFF - 0x44C2
    MODBUS 0xFFFF, 0x0000 - 0x4B37
    USB    0xFFFF, 0xFFFF - 0xB4C8

    Name:   CRC-16/ARC, CRC-16/CMS, CRC-16/DDS-110, CRC-16/MAXIM-DOW, CRC-16/MODBUS, CRC-16/UMTS, CRC-16/USB
    Width:  16
    Poly:   0x8005    x^16 + x^15 + x^2 + 1
    Init:   0xFFFF
    Revert: False
    XorOut: 0x0000
    Check:  0x4B37 ("123456789") size = 9
    MaxLen: 4095 bit

 * @param src
 * @param len
 * @param crc
 * @return
 */
U16 CRC16_MODBUS( void *src, U32 len, U16 crc ); //TODO not work properly


/**
 * @brief CRC16_CITT
    Name:   CRC 16/CITT
    Width:  16
    Poly:   0x1021    x^16 + x^12 + x^5 + 1
    Init:   0xFFFF
    Revert:  False
    XorOut: 0x0000
    Check:  0x29B1 ("123456789") size = 9
    MaxLen: 4095 bit

 * @param src
 * @param len
 * @param crc
 * @return
 */
U16 CRC16_CITT( void *src, U32 len, U16 crc );


/**
 * @brief CRC32_PKZIP
    Name  : CRC-32/ISO-HDLC, CRC-32/ADCCP, PKZIP, Ethernet, 802.3
    Width : 32
    Poly  : 0x04C11DB7    x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11
                       + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
    Init  : 0xFFFFFFFF
    Revert : True
    XorOut: 0xFFFFFFFF
    Check : 0xCBF43926 ("123456789") size = 9
    MaxLen: 268435455 bit, 33554431 bytes

 * @param src
 * @param len
 * @param crc
 * @return
 */
U32 CRC32_PKZIP( void *src, U32 len, U32 crc );


U32 CRC32_CCITT( void *src, U32 len, U32 crc );


/**
 * @brief CRC_UNI
    test CRC UNI
    crc16 = CRC_UNI( &buf[0], size, 16, 0xA001, 0xFFFF, 1, 0 );
    crc16 = CRC_UNI( &buf[0], size, 16, 0x1021, 0, 0, 0 );
    crc32 = CRC_UNI( &buf[0], size, 32, 0x4C11DB7, 0xFFFFFFFF, 0, 0xFFFFFFFF );
    WARNING! very slow
 * @param src
 * @param len
 * @param width
 * @param poly
 * @param init
 * @param dir
 * @param XorOut
 * @return
 */
U32 CRC_UNI( void *src, U32 len, U8 width, U32 poly, U32 init, BOOL dir, U32 XorOut );


void _crc_test( void );


#ifdef	__cplusplus
}
#endif

#endif	/* _CRC_H */
