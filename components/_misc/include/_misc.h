/**
 * @file    misc.h
 * @author  s.aksenov, Ht3h5793, CD45
 * @date    13.01.2013
 * @version V2.0.0
 * @brief   format convertors, str functions, etc
 * @todo
 *
 Вычисление числа пи
int a=10000,b,c=2800,d,e,f[2801],g;main(){for(;b-c;)f[b++]=a/5;
for(;d=0,g=c*2;c-=14,printf("%.4d",e+d/a),e=d%a)for(b=c;d+=f[b]*a, f[b]=d%--g,d/=g--,--b;d*=b);}

 */

#ifndef _MISC_H
#define	_MISC_H 20180922


/**
 *  Раздел для include ---------------------------------------------------------
 */
#include "board.h"
#include <math.h>
#include "_dsp.h"

/**
 *  Раздел для define-----------------------------------------------------------
 */



#ifdef	__cplusplus
extern "C" {
#endif

// TEA =========================================================================
/*
https://ru.wikipedia.org/wiki/TEA

    @value - 64-bit block of data
    @key - 128-bit key

    U32 TEA_data[2] = { 435, 333 };
    U32 TEA_key[4] = { 12332, 2348367676, 666, 474*33/332%33381271+3723888888*212663427/4*312166 };

    cipher_TEA_encrypt( TEA_data, TEA_key );
    cipher_TEA_decrypt( TEA_data, TEA_key );
*/
/**
 * @brief cipher_TEA_encrypt
 * @param value
 * @param key
 */
void cipher_TEA_encrypt( U32 *value, U32 *key );


/**
 * @brief cipher_TEA_decrypt
 * @param value
 * @param key
 */
void cipher_TEA_decrypt( U32 *value, U32 *key );



/*
https://ru.wikipedia.org/wiki/RTEA

    @value - 64-bit block of data
    @key - 256-bit key

U32 RTEA_data[2] = { "12345678" }; //64bit
U32 RTEA_key[8] = { "pass12345678#$@!" };
*/
/**
 * @brief cipher_RTEA_encrypt
 * @param value
 * @param key
 */
void cipher_RTEA_encrypt( U32 *value, U32 *key );


/**
 * @brief cipher_RTEA_decrypt
 * @param value
 * @param key
 */
void cipher_RTEA_decrypt( U32 *value, U32 *key );



/**
 * @brief hashLy - http://habrahabr.ru/post/219139/
 * @param str
 * @return
 */
U32 hash_Ly( char *str );

U32 hash_Rs( const char *str );

U32 hash_Rot13( const char *str );

U32 hash_FAQ6( const char * str );




// coder Gray ==================================================================
/*
 * U32 x;
    for( U32 i = 0; i < 16; i++ )
    {
        x = coder_Gray_encode( i );
        x = coder_Gray_decode( x );
    }
 */
/**
 * @brief coder_Gray_encode
 * @param x
 * @return
 */
U32 coder_Gray_encode( U32 x );


/**
 * @brief coder_Gray_decode
 * @param x
 * @return
 */
U32 coder_Gray_decode( U32 x );


/**
 * @brief sqrt_fixed
 * @param x
 * @return
 */
S32 sqrt_fixed( S32 x );


// FIXED arifmetic =============================================================
#ifndef FIXED
#define FIXED     S32
#endif
#if( FIXED == S32 )
#define FIXED_S     16
#endif

inline FIXED S32_to_fixed( S32 value )
{
    return (value << FIXED_S);
}

inline S32 fixed_to_S32( FIXED  value )
{
    if( value < 0 )
        return ((value >> FIXED_S) - 1);
    else //if(value >= 0) 
        return (value >> FIXED_S);
}

// записывает отношение (a / b) в формате чисел с фиксированной точкой
inline FIXED frac_to_fixed( S32 a, S32 b )
{
    return (a << FIXED_S) / b;
}

// округление до ближайшего целого
inline S32 round_fixed( FIXED value )
{
    return fixed_to_S32( ( value + 5 ) << 15);
}

// представляет число с плавающей точкой в формате чисел с фиксированной точкой
// здесь происходят большие потери точности
inline FIXED double_to_fixed( DOUBLE value )
{
    return (FIXED)round( value * 65536.0 );
}

inline FIXED float_to_fixed( FLOAT value )
{
    return (FIXED)roundf( value * 65536.0 );
}

typedef struct {
    S32 x;
    S32 y;
} vector_2D_S32_t;

typedef struct {
    FLOAT x;
    FLOAT y;
} vector_2D_FLOAT_t;

typedef struct {
    DOUBLE x;
    DOUBLE y;
} vector_2D_DOUBLE_t;

typedef struct {
    S32 x;
    S32 y;
    S32 z;
} vector_3D_t;

#define vector_set(a,b)     a[0]=b[0];a[1]=b[1];a[2]=b[2]
#define vector_cset(a,b)    a[0]=b;a[1]=b;a[2]=b
#define vector_add(a,b)     a[0]+=b[0];a[1]+=b[1];a[2]+=b[2]
//#define _vector_len(a)       sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2])

inline U32 vect_lenght( vector_2D_S32_t *v )
{
    return _dsp_sqrtU32( (U64)( v->x * v->x + v->y * v->y ));
}

static inline FLOAT vect_len_FLOAT( vector_2D_FLOAT_t *v )
{
    return sqrtf( v->x * v->x + v->y * v->y );
}

#define dir_vect_x(dir,acc) (( acc * cos_fixed( dir )) / 0xFFFF )

static inline FLOAT _vect_x_FLOAT( FLOAT len, FLOAT dir )
{
    return len * cosf( dir * DEG2RAD );
}

#define dir_vect_y(dir,acc) (( acc * -sin_fixed( dir )) / 0xFFFF )

static inline FLOAT _vect_y_FLOAT( FLOAT len, FLOAT dir )
{
    return len * sinf( dir * DEG2RAD );
}



/** correct angle, must process after all! */
S32 dir_correct( S32 dir );

static inline FLOAT wrap360( FLOAT deg )
{
	if( deg < 0.0 )
        return 360.0 - _abs( deg );
	else if( deg >= 360.0 )
        return _abs( deg ) - 360.0;
	else
		return deg;
}

// Sorting algorithms ==========================================================

/**
 * @brief sort_Shell_16b
 * @param p
 * @param n
 */
void sort_Shell_16bit( U16 *p, U32 n );

void sort_Shell_32bit( S32 *p, U32 n );


// Converters common use =======================================================

U32 bit_32_reverse( U32 value );

U32 bit_reverse( U32 bits_num, U32 value );

//return hex number in range 0-0xf or error 0xff if char is not a hexadecimal digit
//U8 char2hex (char c);

/**
 * @brief bcd8_to_dec8
 * @param val
 * @return
 */
U8 bcd8_to_dec8( U8 val );


/**
 * @brief dec8_to_bcd8
 * @param val
 * @return
 */
U8 dec8_to_bcd8( U8 val );


/**
 * @brief dec16_to_bcd32
 * @param dec
 * @return
 */
U32 dec16_to_bcd32( U16 dec );

/**
 * @brief char_to_hex - return hex number in range 0-0xf
 *  or error 0xff if char is not a hexadecimal digit
 * @param c
 * @return
 */
U8 char_to_hex( char c );


FAST_U8 U32_to_str( U32 value, char *pstring );

U32 _atoi( char *pstr );

#ifndef BOARD_PC
void _itoa( U32 num, char *pstr );
#endif

int32_t float2int( char *ptr, float number, U8 zeros );


/**
 * @brief _memcpy
 * @param dst — адрес буфера назначения
 * @param src — адрес источника
 * @param n — количество байт для копирования
 * @return
 */
void *_memcpy( void *dst, const void *src, INT n );


/**
 *
 */
void *_memset( void *dst, U8 v, INT n );


/**
 * @brief _strlen - str lenght
 * @param str - pointer of data
 * @return - from 0
 */
U32 _strlen( const char *str );


/**
 * @brief _strnlen
 * The strnlen() function returns the number of bytes in the string pointed to
 * by s, excluding the terminating null bye ('\0'), but at most maxlen.
 * In doing this, strnlen() looks only at the first maxlen bytes at s and never beyond s+maxlen.
 * @param str
 * @param maxlen
 * @return
 */
U32 _strnlen( const char *str, U32 maxlen );


/**
 *
 */
/**
 * @brief str_UTF_to_ANSI - UTF-8 to ANSI coder, src will be replaced by a
 * shorter string, ending 0 terminal
 * @param src
 * @return
 * @note WARNING!, only 2byte coding, other not implemented !
 */
INT str_UTF_to_ANSI( char *src );


/**
 * @brief char_to_lower_case
 * @param ch
 * @return
 */
char char_to_lower_case( char ch );


/**
 * @brief char_to_upper_case
 * @param ch
 * @return
 */
char char_to_upper_case( char ch );

/**
 * @brief str_to_lower_case
 * @param str
 */
void str_to_lower_case( char *str );


/**
 * @brief str_to_upper_case
 * @param str
 */
void str_to_upper_case( char *str );


/**
 * @brief str_remove_spaces
 * @param str
 */
void str_remove_spaces( char *str );



char *_strtok_r( char *str, const char *delim, char **last);


/**
 * @brief _strcmp
 * @param strA
 * @param strB
 * @return -
        Меньше 0	str1 меньше, чем str2
        0	str1 равна str2
        Больше 0	str1 больше, чем str2
 */
S32 _strcmp( const char *str1, const char *str2 );


char *_strncmp( const char *str1, const char *str2, U32 size );



/* Function for scale gray image
 * pixelsIn - 1-sized 2x array for vaules
 * pixelsOut - 1-sized 2x array for vaules
 * w,h - original image
 * w2,h2 - sized image
 */
INT resize_bilinear( S32 *pixelsIn, S32 *pixelsOut, int w, int h, int w2, int h2);


/**** These are needed to transmit and receive ****/

// Given a byte to transmit, this returns the parity as a nibble - 4-bit value
U8 DL_HammingCalculateParity128(U8 value);

// Given two bytes to transmit, this returns the parity
// as a byte with the lower nibble being for the first byte,
// and the upper nibble being for the second byte.
U8 DL_HammingCalculateParity2416(U8 first, U8 second);



/**** These are needed only to receive ****/

// Given a pointer to a received byte and the received parity (as a lower nibble),
// this calculates what the parity should be and fixes the recevied value if needed.
// It returns the number of bits corrected:
// 0 means no errors
// 1 means one corrected error
// 3 means corrections not possible
U8 DL_HammingCorrect128(U8 *value, U8 parity);

// Given a pointer to a first value and a pointer to a second value and
// their combined given parity (lower nibble first parity, upper nibble second parity),
// this calculates what the parity should be and fixes the values if needed.
// It returns the number of bits corrected:
// 0 means no errors
// 1 means one corrected error
// 2 means two corrected errors
// 3 means corrections not possible
U8 DL_HammingCorrect2416(U8 *first, U8 *second, U8 parity);



FLOAT pressure_to_altitude( FLOAT sea_level_Pa, S32 pressure_Pa );

FLOAT mmh_to_Pa( FLOAT mmh );

/**
 * @brief _misc_test
 */
void _misc_test( void );


#ifdef	__cplusplus
}
#endif


#endif	/* _MISC_H */
