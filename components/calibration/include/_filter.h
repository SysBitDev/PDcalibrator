/* 
 * File:   
 * Author:
 *
 * Created on 29 ���� 2013 �., 11:00
 
 http://we.easyelectronics.ru/Soft/sigma-tochechnyy-filtr-kalmana.html
 */

#ifndef FILTERS_H
#define	FILTERS_H 20160702

#include "board.h"


typedef struct _filter_Kalman_t {
	FLOAT X0; // predicted state
    FLOAT P0; // predicted covariance
    
    FLOAT F; // factor of real value to previous real value
    FLOAT H; // factor of measured value to real value
    FLOAT Q; // measurement noise
    FLOAT R; // environment noise
    
    FLOAT state;
    FLOAT covariance;
} filter_Kalman_t;


typedef struct filter_window_32b_t_ {
    S64 tmp;
    S32 window;
} filter_window_32b_t;

typedef struct filter_window_FLOAT_t_ {
    FLOAT tmp;
    FLOAT window;
} filter_window_FLOAT_t;



#ifdef	__cplusplus
extern "C" {
#endif

void filter_Kalman_init( filter_Kalman_t *pf, FLOAT f, FLOAT h, FLOAT q, FLOAT r );

void filter_Kalman_set_state( filter_Kalman_t *pf, FLOAT _state, FLOAT _covariance );

void filter_Kalman_init_default( filter_Kalman_t *pf );

FLOAT filter_Kalman( filter_Kalman_t *pf, FLOAT value );


// Filters =====================================================================
/**
 * @brief filter_arifm - arifmetic filter
 * @param adc_buf
 * @param size
 * @return
 */
U16 filter_16bit_arithmetic_mean( U16 *adc_buf, U32 size );



S32 filter_arifm32( S32 *adc_buf, U32 size, S32 sample );


/**
 * @brief filter_median
 * @param adc_buf
 * @param size
 * @param sample
 * @param tmp_buf
 * @return
 */
U16 filter_median16( U16 *adc_buf, U32 size, U16 sample, U16 *tmp_buf );

S32 filter_median32( S32 *adc_buf, U32 size, S32 sample, S32 *tmp_buf );


S32 filter_window_S32( filter_window_32b_t *pf, S32 ns );

FLOAT filter_window_FLOAT( filter_window_FLOAT_t *pf, FLOAT ns );


#ifdef	__cplusplus
}
#endif

#endif	/* FILTERS_H */

