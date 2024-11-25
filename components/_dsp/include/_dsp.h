/**
 * @file    crypto.h
 * @author  Антон Логинов, Ht3h5793, CD45
 * @date    12.10.2015  15:18
 * @version V1.0.0
 * @brief   
 * @todo 
 
	https://github.com/maniacbug/StandardCplusplus

         
   добавить сжаьтие
     http://we.easyelectronics.ru/Soft/szhatie-zvuka-v-ima-adpcm.html
     

       https://habrahabr.ru/post/247385/
       
       

         
         https://github.com/dccharacter/AHRS

   
 */

#ifndef _DSP_H
#define	_DSP_H 20230118

/** include */
#include "board.h"
#include <math.h>

/** defines */

/** typedefs */


#ifdef	__cplusplus
extern "C" {
#endif


// http://cdeblog.ru/post/user-sqrt/
/*! Функция usr_sqrt(x) ? Вычисление квадратного корня.
* \param U32 value ? число квадратный корень которого нужно вычислять
* \return U32 значение квадратного корня
 */
U32 _dsp_sqrtU32( U32 value );


typedef enum {
	FT_DIRECT                      = -1,    // Direct transform.
	FT_INVERSE                     = 1    // Inverse transform.
} DSP_FFT_DIRECTION_t;


void _dsp_dft( INT n, DSP_FFT_DIRECTION_t inverse, FLOAT *gRe, FLOAT *gIm, FLOAT *GRe, FLOAT *GIm );


void _dsp_fft( INT n, DSP_FFT_DIRECTION_t inverse, FLOAT *gRe, FLOAT *gIm, FLOAT *GRe, FLOAT *GIm );


BOOL  _dsp_FFT_FLOAT( FLOAT *Rdat, FLOAT *Idat, int N, int LogN, DSP_FFT_DIRECTION_t Ft_Flag );



/**
http://ru.wikipedia.org/wiki/Direct_Digital_Synthesizer

Здесь dph — приращение фазы, phase — текущая (мгновенная) фаза,
amp — текущая (мгновенная) амплитуда синтезированного 
гармонического сигнала. Если функция next_amp вызывается с
тактовой частотой Fc, то ее возвращаемые значения будут
представлять собой выборки синусоидального сигнала с частотой
Fc*dph/2^{32} и амплитудой 511. Эта амплитуда соответствует 
диапазону входных значений 10-разрядного ЦАП. Здесь также 
использовано свойство периодичности функции синуса, а именно 
тот факт, что при переполнении аккумулятора фазы phase, его 
значение изменяется на 232, а аргумент синуса — на 2?, что не
влияет на результат.
*/



// sin/cosin functions =========================================================

#define RANGE                           (0xFFFF)

/**
 * sin/cos table in fixed value
 * angle - 0-359
 * out - 0-65565
 */
/**
 * @brief sin_fixed
 * @param angle
 * @return
 * @note	Be careful with this
 */
S32 CODE sin_fixed( S32 angle );


/**
 * @brief cos_fixed
 * @param angle
 * @return
 */
S32 CODE cos_fixed( S32 angle );


S32 _dsp_avg( S32 *pbuf, UINT len );



RET _dsp_sint_sin( S32 *pbuf, U32 period_size, U32 ampl, S32 adder );

RET _dsp_sint_cos( S32 *pbuf, U32 period_size, U32 ampl, S32 adder );



/* get adder value for DDS mode sintezator
 *
 */
U32 _dsp_dds_sint_adder( U32 sampling_rate, U32 freq );
// ( 32 - SIN_TABLE_BIT_SIZE )fetch and output sine sample
#define _dsp_dds_sint_get(rom_sin,DDSp,DDSd)   rom_sin[ DDSp >> 24 ]; DDSp = DDSp + DDSd;



// https://habr.com/ru/post/432498/
RET _dsp_goertzel_init( U32 sampling_rate, U32 freq, S8 *psin_buf, S8 *pcos_buf, U32 size, U32 ampl );
S64 _dsp_goertzel_run( S8 *psin_buf, S8 *pcos_buf, S32 *pbuf, U32 size, U32 scaling );

U64 _dsp_goertzel( U32 sampling_rate, U32 freq, S32 *pbuf, U32 size, U32 ampl, U32 scaling );
FLOAT _dsp_goertzel_FLOAT( S32 *pbuf, U32 Fs, U32 freq, U32 N, U32 ampl, U32 S );
S32 _dsp_goertzel_S32( S16 *pbuf, S32 Fs, S32 Ft, INT N, U32 b );


void _dsp_test( void );



#ifdef	__cplusplus
}
#endif

#endif	/* _DSP_H */
