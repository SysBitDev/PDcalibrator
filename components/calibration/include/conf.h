/*
 * File:   conf.h
 * TODO list:
 * Сделать шв всем одинаковый только для USB, что бы ренумерации не было постоянно
 *
 */

#ifndef CONF_H
#define	CONF_H 20240226


#include "board.h"
#include "common.h"
#include "build_defs.h"


// common control ==============================================================
#define TEST_ALL						0 // !!! <- only test mode!

#define UART_PROTOCOL_MAVLINK			1




//#define TEST_CLI						0
#define SERVO_START_TEST				0

#define	COMMON_NEED_MAG					1
#define	COMMON_NEED_BARO				1
#define	COMMON_NEED_GPS					1

#define MODE_EBIMBA						1
//#define MODE_ROCKETA					0
#define MODE_CALIBRATE_AND_LOGS			1

//#define IMU_SAMPLE_RATE 				(833)//103) // replace this with actual sample rate
#define IMU_SAMPLE_RATE 				(103)

#define HAL_USB_TX_BUF_SIZE				(512)



#if( MODE_EBIMBA && UART_PROTOCOL_MAVLINK )
#define DEV_CONFIG_STRING				"EBIMBA+MAVLINK"
#else
#define DEV_CONFIG_STRING				"EBIMBA+PARSER"
#endif

// Mavlink =====================================================================
//#if UART_PROTOCOL_MAVLINK

#define MAV_DEFAULT_TARGET_SYS			1
#define MAV_DEFAULT_TARGET_COMPONENT 	1
#define SYSTEM_ID						255
#define COMPONENT_ID					0

//#else
// modParcer ===================================================================
#define PIK_ADR_DEFAULT          		((U8)'#')
#define MODPARCER_CRC_CHECK             1
#define PIK_START                  		0x0D
#define PIK_FIN                     	0x0A
#define PARSER_MAX_DATA_SIZE        	(512)

//#endif


typedef struct PACK_START {
	U16 max;
	U16 rssi[ 12 ];
	S16 snr[ 12 ];
} fd_t __attribute__ ((aligned (sizeof(U16))));

typedef struct PACK_START {
	U8 mode;
	U8 ch_num;
	U16 rssi[ 12 ];
	S16 snr[ 12 ];
	FLOAT angl_x;
	FLOAT angl_y;
} fd_clb_t __attribute__ ((aligned (sizeof(U16))));


// for board voltage mon =======================================================
#define ADC_SAMPLES_NUM              	2//8 //2 minimum
#define ADC_SAMPLING_RATE               ( 1000 ) //1Ks/S

#define SYS_SETTINGS_IN_FLASH			1

#if SYS_SETTINGS_IN_FLASH
// Flash address for store settings
//FLASH_BASE + 128K - 2K = 126K
#define FLASH_SETTINGS_ADDRESS			( FLASH_BASE + ( 128 * 1024 - 2048 ))//0x0801F800 )
#else
#define FLASH_SETTINGS_ADDRESS			0
#endif

typedef enum {
	STATE_IDLE							= 0,
	STATE_ZERO_G_WAIT 					= 5,
	STATE_OPERATE 						= 10,
	STATE_CALIBRATE						= 20,
	STATE_CALIBRATE_AUTO				= 21,
	STATE_CALIBRATE_CROSS				= 22,

	STATE_GET_ID						= 30,
	STATE_GET_LOGS						= 40,
	STATE_LOGS_ERASE					= 50,
	STATE_INTERNAL_ERROR				= 60,

	STATE_FUSE_ACTIVATE 				= 88,
	STATE_SELF_DESTRUCT					= 666,

} STATE_enum;

typedef enum {
	SERVO_1								= 0,
	SERVO_2								= 1,
	SERVO_3								= 2,
	SERVO_4								= 3,
	SERVO_NUM_MAX						= 4
} SERVO_enum;

typedef struct PACK_START {
	S16 max;
	S16 min;
	S16 offset;
} servo_t __attribute__ ((aligned (sizeof(U16))));

typedef struct PACK_START {
	U32 serial_FD_baudrate;
	U32 serial_GPS_baudrate;
	U32 serial_EXT_baudrate;

	U32 ADC_VREF;
	FLOAT ADC_divider;
	S32 board_VBAT_mV_min;
	S32 board_VBAT_mV_max;

	SYSTIME imu_delay;

	FLOAT acc_bias_x;
	FLOAT acc_bias_y;
	FLOAT acc_bias_z;
	FLOAT acc_scale_x;
	FLOAT acc_scale_y;
	FLOAT acc_scale_z;
	BOOL acc_calibrated;

	FLOAT gyr_bias_x;
	FLOAT gyr_bias_y;
	FLOAT gyr_bias_z;
	FLOAT gyr_scale_x;
	FLOAT gyr_scale_y;
	FLOAT gyr_scale_z;
	BOOL gyr_calibrated;

	FLOAT mag_bias_x;
	FLOAT mag_bias_y;
	FLOAT mag_bias_z;
	FLOAT mag_scale_x;
	FLOAT mag_scale_y;
	FLOAT mag_scale_z;
	BOOL mag_calibrated;

	BOOL all_calibrated;

	S32 acc_max_value;

	FLOAT sea_level_Pa;

	U8 servo_num;
	U8 servo_type;
	servo_t servo[ SERVO_NUM_MAX ];

	BOOL PID_on;
	FLOAT P;
	FLOAT I;
	FLOAT D;
	SYSTIME dt;

	S32 servo_angle_max;
	S32 servo_angle_min;

	SYSTIME log_write_delay;
	U32 log_rec_addr;
	U16 log_rec_num;

	U32 writes;

	U32 build_time;
	U32 build_date;
	U32 board_number;

	U32 crc;
} settings_t __attribute__ ((aligned (sizeof(U16)))); // must be 16bit size !


typedef enum {
	EXT_CONNECTION_ESTABLISHED 			= 0,
	EXT_CONNECTION_NOT_FOUND 			= 1,

} EXT_CONNECTION_enum;

#pragma pack(push,1) //flags
typedef struct {
	UINT acc_calibrated:1;
	UINT gyr_calibrated:1;
	UINT mag_calibrated:1;
	UINT fd_status:1;
	UINT imu_status:1;
	UINT bar_status:1;
	UINT mag_status:1;
	UINT gps_status:3;
	UINT flash_crc_status:1;
	UINT flash_status:1;
	UINT calibrate_status:1;
	UINT security_status:1;
	UINT integrity_status:1;
	UINT fuze_status:1;
	UINT external_connection_status:8;  //serial to drone\charger

	UINT _tmpbits:8;
} flags_t;
#pragma pack(pop)

typedef struct {
	INT state;
	U16 error;
	U32 error_cnt;
	flags_t flags;
	//U32 timestamp;
	BOOL log_on;

	S16 board_VBAT_mV;
	//S16 imu_temperature; //in 1 Celsius
	U32 serial_msg_rx_cnt;

	FLOAT mcu_temperature;

	// GPS
	U32 gps_points_num;
	FLOAT gps_lon; //X
	FLOAT gps_lat; //Y
	FLOAT gps_alt; //Z
	U32 gps_time;
	U32 gps_date;
	U8 gps_sat_num;

    FLOAT fd_dir[ 2 ];

	SYSTIME imu_last_update;
	//S16 imu_temp;
	S16 ax, ay, az;
	S16 gx, gy, gz;
	S16 mx, my, mz;

	FLOAT imu_temperaturef;
	FLOAT fax, fay, faz;
	FLOAT fgx, fgy, fgz;

	FLOAT mag_temperature;
	FLOAT fmx, fmy, fmz;

	FLOAT bar_temperature;
	FLOAT bar_pressure;
	FLOAT bar_alt;

	FLOAT gnd_alt;

	S16 servo_val[ SERVO_NUM_MAX ]; //-1000 +1000
	S16 servo_enc[4];

	U32 log_rec_addr;
} sys_t;


typedef struct  { // IMU
	FLOAT gfax, gfay, gfaz;

	FLOAT roll_rad; //roll - a circular (clockwise or anticlockwise) movement of the body as it moves forward
	FLOAT pitch_rad; //pitch - nose up or tail up.
	FLOAT yaw_rad; //yaw - nose moves from side to side

	FLOAT roll; //roll - a circular (clockwise or anticlockwise) movement of the body as it moves forward
	FLOAT pitch; //pitch - nose up or tail up.
	FLOAT yaw; //yaw - nose moves from side to side

	//	//in grd
	U8 stationary;

	FLOAT delta_time;

	FLOAT velocity_x; //X
	FLOAT velocity_y; //Y
	FLOAT velocity_z; //Z

	// Local coord sys
	FLOAT pos_x; //X
	FLOAT pos_y; //Y
	FLOAT pos_z; //Z

	// Global coord sys
	FLOAT pos_X; //X
	FLOAT pos_Y; //Y
	FLOAT pos_Z; //Z

} ahrs_t;

// calibrations ================================================================
#pragma pack(push,1)
typedef struct DATA_PACK {
	U32 MCUID[3];
	CHAR board_name[32];
	CHAR device_config_str[32];
	U32 build_date; //BUILD_DATE
	U32 build_time; //BUILD_TIME
	U32 board_number;
	U32 flash_size;
	U32 log_rec_addr;
	U32 log_write_delay;
    U16 error;
    flags_t flags;
} mcu_to_pc_id_data_t;
#pragma pack(pop)

typedef struct {
	INT state;

    U32 fd_val[ 12 ];
    U32 fd_snr[ 2 ];
    FLOAT fd_dir[ 2 ];

	FLOAT temperature;
	FLOAT pressure;
	FLOAT bar_alt;

	S16 ax, ay, az;
	S16 gx, gy, gz;
	S16 mx, my, mz;
	S16 board_VBAT_mV;

	FLOAT roll; //roll - a circular (clockwise or anticlockwise) movement of the body as it moves forward
	FLOAT pitch; //pitch - nose up or tail up.
	FLOAT yaw; //yaw - nose moves from side to side

	FLOAT velocity_x;
	FLOAT velocity_y;
	FLOAT velocity_z;

	// IMU
	FLOAT imu_x;
	FLOAT imu_y;
	FLOAT imu_z;

	S16 servo_val[ SERVO_NUM_MAX ];
    S16 servo_enc[ 4 ];

} mcu_to_pc_clb_data_t;

typedef struct {
    U8 cmd;
    FLOAT acc_bias[3];
    FLOAT acc_scale[3];
    FLOAT gyr_bias[3];
    FLOAT gyr_scale[3];
    FLOAT mag_bias[3];
    FLOAT mag_scale[3];
    FLOAT sea_level_hPa;
	servo_t servo[ SERVO_NUM_MAX ];

	U32 board_number;
} pc_to_mcu_clb_data_t;


#pragma pack(push,1)
typedef struct log_record_t_ { //size only 256 byte!
	U32 num; //number of this record

	INT state;
	U16 error;
	flags_t flags;

	//PWR
	U16 board_VBAT_mV;

	// RC_CHANELS - FD
	U32 fd_v[ 12 ];
	U32 fd_snr[ 2 ];
	FLOAT fd_dir[ 2 ];

	//U16 Vservo;
	S16 servo_val[ SERVO_NUM_MAX ]; //-1000 +1000

	// IMU
	//U64 timestamp;
	S16 ax, ay, az;
	S16 gx, gy, gz;
	S16 mx, my, mz;

	FLOAT fax, fay, faz;
	FLOAT fgx, fgy, fgz;
	FLOAT fmx, fmy, fmz;

	FLOAT temperature_IMU;

	// GPS
	FLOAT gps_lon; //X
	FLOAT gps_lat; //Y
	FLOAT gps_alt; //Z
	U32 gps_points_num;
	U32 gps_time;
	U32 gps_date;

	// IMU
	FLOAT imu_roll; //roll - a circular (clockwise or anticlockwise) movement of the body as it moves forward
	FLOAT imu_pitch; //pitch - nose up or tail up.
	FLOAT imu_yaw; //yaw - nose moves from side to side

	FLOAT velocity_x;
	FLOAT velocity_y;
	FLOAT velocity_z;

	FLOAT imu_x;
	FLOAT imu_y;
	FLOAT imu_z;

	U32 timestamp_save;

	S32 pressure;
	FLOAT bar_alt;

	U32 tmp[9]; //filler

	U16 crc; //not remove!
} log_record_t;
#pragma pack(pop)


//#define MEMORY_ADR_MAX					( 1024 * 1024 * 1 ) //max address 1M
#define LOG_ADR_START					(4096)
//#define LOG_WRITE_DELAY					( 10 ) //in ms


//#define set_servo_x						set.servo[0]
//#define set_servo_y						set.servo[1]

#ifdef	__cplusplus
extern "C" {
#endif

void hal_init_all( void );
void drv_init_all( void );
void sys_us_tic_clr();
U32 sys_us_tic_get();

void drvGPIO_external_interrupt_init( void );
//void EXTI0_1_IRQHandler( void );
//void process_data( U16 *pbuf );

// link ========================================================================

RET serial_USB_send( U8 *p );
BOOL serial_USB_recive( U8 *p, INT size );
BOOL halUSB_CDC_puts( U8 *buf, U32 size );
void halUSB_CDC_run( void );


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
void serial_USB_print( const char *format, ...);
//#define  serial_USB_print(f,...) do{ xsprintfs( USB_tx_buffer, f, ... ); serial_USB_send( USB_tx_buffer ); }while(0)


// servo set angle\direction
void servo_dir_set( SERVO_enum num, INT angle );


void settings_to_default( void );
RET settings_write( void );
void settings_init( void );

RET log_read( U32 addr, log_record_t *p );
RET log_write( U32 addr, log_record_t *p );
RET log_erase_all( void );
RET log_rescan( void );
BOOL log_status_get( void );

void start_record( void );
void stop_record( void );



U32 drv_ADC_VIN_get( void );
void drv_bar_get( sys_t *s );

void imu_calibrated_get( S16 *raw_val, FLOAT *bias, FLOAT *scale );

RET drv_mag_get( sys_t *sys );
RET drv_imu_get ( sys_t *sys );


// processors ==================================================================
void callback_imu_ready( void );

void process_external_serial( void );
void process_mag_sensor( void );
void process_adc( void );
void process_imu_sensor( void );
void process_pressure_sensor( void );
void process_gps( void );
void process_sensors( void );
void process_fd_serial( void );
void process_calibrate( void );
void process_servo( void );
void process_logs( void );
void process_usb( void );
void process_sys( void );

#define PROCESS_TEST PIO_TEST1_INV //tic = GET_CORE_COUNT; RESET_CORE_COUNT ////LED_TEST_INV;


#ifdef	__cplusplus
}
#endif

#endif	/* CONF_H */
