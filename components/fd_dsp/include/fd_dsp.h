/**
 * @file    dsp_fd.h
 * @author  Ht3h5796-12
 * @date    22.12.2022  8.19
 * @version V1.0.1
 * @brief
 */

#ifndef FD_DSP_H
#define	FD_DSP_H 20240425

/** include */
#include "board.h"


/**
* Configuration:
* Defines the size of the ADC sample array. Defining a larger value here will significantly increase
* the amount of static RAM usage, but more values will be used for averaging, leading to lower noise.
*/
#define DSP_FD_CHANNEL_NUM_MAX			12UL /* Number of channel maximum */

#ifndef DSP_FD_CHANNEL_NUM
#define DSP_FD_CHANNEL_NUM				4UL /* Number of channel */
#endif

#ifndef DSP_FD_SAMPLING_RATE
#define DSP_FD_SAMPLING_RATE       		4000UL // 4000
#endif

//#define DSP_FD_FRQ_NUM					2UL
#ifndef DSP_FD_FRQ_NUM
#define DSP_FD_FRQ_NUM					2UL //4 max /* for analysis for SNR */
#endif

#ifndef DSP_FD_BUF_SIZE
#define DSP_FD_BUF_SIZE               	1500UL // 128
#endif

#ifndef GOERTZEL_MASHTAB
#define GOERTZEL_MASHTAB                3
#endif

#define GOERTZEL_v1						1 //old variant
//#define GOERTZEL_v2						1
//#define GOERTZEL_v3						1

typedef struct DATA_PACK {
    U32 v[ DSP_FD_CHANNEL_NUM_MAX * DSP_FD_FRQ_NUM ];
    U32 rssi[ DSP_FD_CHANNEL_NUM_MAX * DSP_FD_FRQ_NUM ];
    U32 snr[ DSP_FD_CHANNEL_NUM_MAX ];
    FLOAT x[2];
    FLOAT y[2];
    FLOAT dir[2];
    FLOAT len[2];
    FLOAT fx[2];
    FLOAT fy[2];
    FLOAT fpx[2];
    FLOAT fpy[2];
    U8 fd_num;
    U32 bufpos[2];
    //U8 dft_mode; //1 - old, 3 - new
    //U8 vector_mode; //for 3FD, 0 - first, 1 - Yurii, 2 - Pavel
    U16 buf_size; //1500
} fd_data_t;

typedef enum {
	FD_MODE_SELF_TEST 	= 0,
	FD_MODE_RUN			= 1,
	FD_MODE_RESET		= 2,
	FD_MODE_CALIBRATE	= 3,
} FD_MODE;

typedef enum {
    FD_CLB_STATE_INIT               = 0,
    FD_CLB_STATE_NOP                ,
    FD_CLB_STATE_RESET              ,
    FD_CLB_STATE_AFTER_RESET        ,
    FD_CLB_STATE_DATA_COLLECT       ,
    FD_CLB_STATE_DATA_COLLECT_OK    ,
    FD_CLB_STATE_DATA_COLLECT_ERR   ,
    FD_CLB_STATE_WAIT_SEND          ,
    FD_CLB_STATE_CALIBRATE_WRITE    ,
} FD_CLB_STATE;

typedef struct DATA_PACK {
    // status/mode bit field for common use
    UINT version:11;     // version in decimal MMDD MM - mounth, DD - day
    UINT calibrated:1;   // reset - 0, calibrated - 1
    UINT error_code:8;   // error codes, RET_xxx, RET_OK = 0 - no errors
    UINT ch_num:4;       // fotodiode channels number - 1-2-3-4-6
    UINT branches_num:2; // branches for channels 1-3
    UINT vector_mode:2;  // mode, 0 only
    UINT filler1:4;      // filler to 32 bits

    U16 inc_cnt;      // filler to 32 bits

    U32 idnum;
    // processed data
//    U16 raw[1-24]; // 12 (6 fotodiodes) or 8 (4 fotodiode) * frequency number(2)
//    U16 snr[1-12]; // signal to noise ratio - sum of all channels for
//    FLOAT x;       // after calibration - signal coordinates
//    FLOAT y;
    //FLOAT dir;
} fd_serial_data_t;

// for DRAW mode ===============================================================
#ifndef MARK_SIZE
#define MARK_SIZE 15
#endif

#ifndef GRID_SIZE //grid size
#define GRID_SIZE   	17 //grid size
#endif
//#if LCD_LPH9157
//#define GRID_SIZE   	5 //grid size
//#elif LCD_ILI9341
//#define GRID_SIZE   	15 //grid size
//#elif LCD_800480
//#define GRID_SIZE   	32 //grid size
//#else
//#define GRID_SIZE   	17 //grid size
//#endif
#define GRID_N   		5 //grid cells number

#define AREA_SIZE (GRID_SIZE*GRID_N)


#ifndef FD_DSP_CROSS_TYPE
#define FD_DSP_CROSS_TYPE 1
#endif

#ifndef FD_DSP_TRAIN_DOTS
#define FD_DSP_TRAIN_DOTS		90
#endif


//#define FD_CALIBRATE_DOTS       360

#define FD_CLB_DOTS       400

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * @brief dsp_fd_frq_set
 * @param n
 * @param f
 */
void fd_dsp_frq_set( U32 n, FLOAT f );

/**
 * @brief dsp_fd_frq_get
 * @param n
 * @return
 */
FLOAT fd_dsp_frq_get( U32 n );

/**
 * @brief dsp_fd_sample_rate_set
 * @param sr
 */
void fd_dsp_sample_rate_set( FLOAT sr );

/**
 * @brief dsp_fd_sample_rate_get
 * @return
 */
FLOAT fd_dsp_sample_rate_get( void );

/**
 * @brief dsp_fd_buf_size_set
 * @param s
 */
void fd_dsp_buf_size_set( U32 s );

/**
 * @brief dsp_fd_buf_size_get
 * @return
 */
U32 fd_dsp_buf_size_get( void );

/**
 * @brief dsp_fd_mashtab_set
 * @param m
 */
void fd_dsp_mashtab_set( U32 m );

/**
 * @brief dsp_fd_mashtab_get
 * @return
 */
U32 fd_dsp_mashtab_get( void );

/**
 * @brief dsp_fd_init
 */
void fd_dsp_init( void );

/**
 * @brief dsp_fd_process - Gortzel or FFT process
 * @param p
 * @param fdp
 */
void fd_dsp_process_v1( U16 *p, fd_data_t *fdp );
void fd_dsp_process_v3( U16 *p, fd_data_t *fdp );

/**
 * @brief dsp_fd_vector_get - get computed vector to target
 * @param fd
 * @param amp
 * @param tp
 */
void fd_dsp_vector_get( fd_data_t *fd, FLOAT amp );

/**
 * @brief fd_dsp_calibrate_vector_get
 * @param fd
 * @param clb_val
 */
void fd_dsp_calibrate_vector_get( fd_data_t *fd, FLOAT *clb_val );


#if FD_DRAW

/**
 * @brief paint_fd_vector - draw target
 * @param center_x
 * @param center_y
 * @param x
 * @param y
 * @param mark_size
 * @param mark_filled
 */
void fd_dsp_paint_vector( COORD center_x, COORD center_y, COORD x, COORD y, COORD mark_size );

/**
 * @brief dsp_fd_draw - custom draw for MCU board, need test
 * @param pfd_level - pointers to levels from fd
 * @param pfd_vector
 */
void fd_dsp_draw( fd_data_t *pfd_level, U8 mode, U32 mul );
void fd_dsp_draw_( fd_data_t *pfd_level, U8 mode, U32 mul );


/**
 * @brief fd_dsp_draw_init
 * @param pfd_data
 */
void fd_dsp_draw_init( fd_data_t *pfd_data );

/**
 * @brief fd_dsp_add_new_vector
 * @param pfd_level
 */
void fd_dsp_add_new_vector( fd_data_t *pfd_level );

/**
 * @brief fd_dsp_calibrate_process
 * @param pfd_data
 * @param clb
 */
void fd_dsp_calibrate_process( fd_data_t *pfd_data, FLOAT *clb );

/**
 * @brief fd_dsp_draw_all
 * @param pfd_level
 * @param mode
 * @param mul
 */
void fd_dsp_draw_all( fd_data_t *pfd_data, fd_serial_data_t *pfsd, U32 mode, U32 filter, U32 mul );

#include "modGUI.h"
INT fd_dsp_button_test( button_t *b );
int fd_dsp_touch_has_signal( void );
int fd_dsp_touch_touched( void );
int fd_dsp_touch_released( void );
void fd_dsp_buttons_processor( U32 *mode, U32 *filter_mode, U32 *mul );

void fd_dsp_calibrate_reset( void );
void fd_dsp_calibrate_write( void );

#endif //FD_DRAW

/**
 * @brief fd_dsp_polinom
 * @param pdf
 * @return
 */
INT fd_dsp_polinom( fd_data_t *pdf );

/**
 * @brief fd_dsp_tx_packet
 * @param buf
 * @param pfd
 * @param calibrated
 * @return
 */
INT fd_dsp_tx_packet( U8 *buf, fd_data_t *pfd, U8 calibrated );

/**
 * @brief fd_dsp_rx_packet
 * @param buf
 * @param pfsd
 * @param pfd
 * @return
 */
INT fd_dsp_rx_packet( U8 *buf, fd_serial_data_t *pfsd, fd_data_t *pfd );

/**
 * @brief dsp_fd_test
 */
void fd_dsp_test( void );


#ifdef	__cplusplus
}
#endif

#endif	/** FD_DSP_H */

