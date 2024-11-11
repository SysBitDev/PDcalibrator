#include "_filter.h"
#include "_misc.h"

void filter_Kalman_init( filter_Kalman_t *pf, FLOAT f, FLOAT h, FLOAT q, FLOAT r )
{
    pf->F = f;
    pf->H = h;
    pf->Q = q;
    pf->R = r;
}

void filter_Kalman_set_state( filter_Kalman_t *pf, FLOAT state, FLOAT covariance )
{
    pf->state = state;
    pf->covariance = covariance;
}

void filter_Kalman_init_default( filter_Kalman_t *pf )
{
    pf->F = 1;
    pf->H = 1;
    pf->Q = 2;
    pf->R = 15;
    pf->state = 0;
    pf->covariance = 0.5;
}

FLOAT filter_Kalman( filter_Kalman_t *pf, FLOAT value )
{
	FLOAT K;
    //time update - prediction
    pf->X0 = pf->F * pf->state;
    pf->P0 = pf->F * pf->covariance * pf->F + pf->Q;
    //measurement update - correction
    K = pf->H * pf->P0 / (pf->H * pf->P0 * pf->H + pf->R);
    pf->state = pf->X0 + K * (value - pf->H * pf->X0);
    pf->covariance = ( 1.0f - K * pf->H ) * pf->P0;

    return pf->state;
}

//==============================================================================
S32 filter_window_S32( filter_window_32b_t *pf, S32 ns )
{
    S64 t1 = pf->tmp;
    S64 t2 = ns * 100;
    t1 = ( t1 * pf->window );
    t2 = ( t2 * ( 100ULL - pf->window ) );
    t1 = t1 + t2;
    t1 = t1 / 100;
    pf->tmp = t1;
    t1 = (S64)t1 / 100;
#if DEBUG_DSP_FWS32_OVERFLOW
    if( t1 > 0x7FFFFFFF ) //test
    {
        return 0x7FFFFFFF;
    }
#endif
    return (S32)t1;
}

FLOAT filter_window_FLOAT( filter_window_FLOAT_t *pf, FLOAT ns )
{
    pf->tmp = ( pf->tmp * pf->window + ns * ( 100. -pf->window ) ) * 0.01;
    return pf->tmp;
}

U16 filter_16bit_arithmetic_mean( U16 *adc_buf, U32 size )
{
    U32 i, tmp32;

    tmp32 = 0;
    for( i = 0; i < size; i++ )
    {
        tmp32 += adc_buf[ i ];
    }

    return tmp32 / size;
}


U16 filter_arifm16( U16 *adc_buf, U32 size, U16 sample )
{
    U32 i, tmp32;

    U32 ADC_filter_pointer = adc_buf[ size ]; //get pointer

    if( ++ADC_filter_pointer >= size ) //correction pointer
    {
        ADC_filter_pointer = 0;
    }
    adc_buf[ ADC_filter_pointer ] = sample;
    adc_buf[ size ] = ADC_filter_pointer; //store pointer

    tmp32 = 0;
    for( i = 0; i < size; i++ )
    {
        tmp32 += adc_buf[ i ];
    }

    return tmp32 / size;
}


S32 filter_arifm32( S32 *adc_buf, U32 size, S32 sample )
{
    U32 i;
    S64 tmp64;

    U32 ADC_filter_pointer = adc_buf[ size ]; //get pointer

    ADC_filter_pointer++;

    if( ADC_filter_pointer >= ( size - 1) ) //correction pointer
    {
        ADC_filter_pointer = 0;
    }

    adc_buf[ ADC_filter_pointer ] = sample;
    adc_buf[ size ] = ADC_filter_pointer; //store pointer

    tmp64 = 0;
    for( i = 0; i < size; i++ )
    {
         tmp64 += adc_buf[ i ];
    }

    return (S32)(tmp64 / size);
}


U16 filter_median16( U16 *adc_buf, U32 size, U16 sample, U16 *tmp_buf )
{
    U32 tmp32;

    U32 ADC_filter_pointer = adc_buf[ size ]; /*get pointer */

    if( ++ADC_filter_pointer >= size ) /* mov pointer to new position */
    {
        ADC_filter_pointer = 0;
    }
    adc_buf[ ADC_filter_pointer ] = sample;
    adc_buf[ size ] = ADC_filter_pointer; /* store pointer */

    _memcpy( (U8 *)tmp_buf, (U8 *)adc_buf, size * sizeof(U16) ); /* Copy the data */

    sort_Shell_16bit( tmp_buf, size ); /* sorting.. */

    /*  arithmetic sum & central */
    tmp32  = (U32)tmp_buf[ ( size / 2U ) - 2U ];
    tmp32 += (U32)tmp_buf[ ( size / 2U ) - 1U ];
    tmp32 += (U32)tmp_buf[ ( size / 2U ) ];
    tmp32 += (U32)tmp_buf[ ( size / 2U ) + 1U ];
    tmp32 += (U32)tmp_buf[ ( size / 2U ) + 2U ];

    return (U16)(tmp32 / 5);
}


S32 filter_median32( S32 *adc_buf, U32 size, S32 sample, S32 *tmp_buf )
{
    S64 tmp;

    U32 ADC_filter_pointer = adc_buf[ size -1 ]; /*get pointer */

    if( ++ADC_filter_pointer >= size ) /* mov pointer to new position */
    {
        ADC_filter_pointer = 0;
    }
    adc_buf[ ADC_filter_pointer ] = sample;
    adc_buf[ size -1] = ADC_filter_pointer; /* store pointer */

    size = size - 1;

    _memcpy( (U8 *)tmp_buf, (U8 *)adc_buf, size * sizeof(S32) ); /* Copy the data */

    sort_Shell_32bit( tmp_buf, size ); /* sorting.. */

    /*  arithmetic sum & central */
    //tmp32  = (U32)tmp_buf[ ( size / 2U ) - 2U ];
    tmp  = (S64)tmp_buf[ ( size / 2U ) - 1U ];
    tmp += (S64)tmp_buf[ ( size / 2U ) ];
    tmp += (S64)tmp_buf[ ( size / 2U ) + 1U ];
    //tmp32 += (U32)tmp_buf[ ( size / 2U ) + 2U ];

    return (S32)(tmp / 3 );
}


