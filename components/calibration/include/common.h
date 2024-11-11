/*
 * @file
 * @author  Ht3h5793
 * @date    08.09.2013
 * @version V3.5.0
 * @brief
 */

#ifndef COMMON_H
#define COMMON_H                        20210508

#include "board.h"
//#include <inttypes.h>
#include <stdint.h>

/**
 *  for defines ================================================================
 */
/** Standard & common functions return, state indicator */
 enum {
    RET_OK                            	= 0,

    RET_BUSY                            = 1,
    RET_NOT_READY                       = 2,
    RET_READY                           = 3,
    RET_NOT_INIT                        = 4,
    RET_INIT                            = 5,

    RET_ERROR                           = 16, //0x10, // common
    RET_ERROR_UNKNOWN                   = 17, //0x11,
    RET_ERROR_TIMEOUT                   = 18, //0x12,
    RET_ERROR_INDEX_OUT_OF_BOUNDS       = 19, //0x13,
    RET_ERROR_VALUE                     = 20, //0x14,
    RET_ERROR_NULL                      = 21, //0x15,
    RET_ERROR_OVERFLOW                  = 22, //0x16,
    RET_ERROR_STACK_OVERFLOW            = 23, //0x17,
    RET_ERROR_STACK_UNDERFLOW           = 24, //0x18,
    RET_ERROR_OUT_OF_MEMORY             = 25, //0x19,
    RET_ERROR_EXCESS                    = 26, //0x1A, // excess size of string
    RET_ERROR_NOT_FOUND                 = 27, //0x1B,
    RET_ERROR_NOT_RESPOND               = 28, //0x1C,
    RET_ERROR_CRC                       = 29, //0x1D,
    RET_ERROR_DENIED                    = 30, //0x1E,
    RET_ERROR_MALFUNCTION               = 31, //0x1F,
    RET_ERROR_NOT_SUPPORTED             = 32, //0x20,
    RET_ERROR_STATE                     = 33, //0x21,
    RET_ERROR_NOT_INIT                  = 34,
	// hardware errors
    RET_ERROR_OVERHEAT                  = 35,
	RET_ERROR_FREEZE                  	= 36,
	RET_ERROR_UNDERVOLTAGE 				= 37,
	RET_ERROR_OVERVOLTAGE 				= 38,
    RET_ERROR_OSC_STOP                  = 39,


	// for special use ---------------------------------------------------------
    RET_ERROR_EEPROM_NOT_PRESENT        = 0x1000,
    RET_ERROR_EEPROM_WRITE_PROTECTION   = 0x1000,
    RET_ERROR_EEPROM_READ               = 0x1000,
    RET_ERROR_EEPROM_ERASE              = 0x1000,
    RET_ERROR_EEPROM_WRITE              = 0x1000,

    RET_ERROR_FLASH_NOT_PRESENT         = 0x1100,
    RET_ERROR_FLASH_WRITE_PROTECTION    = 0x1101,
    RET_ERROR_FLASH_READ                = 0x1102,
    RET_ERROR_FLASH_ERASE               = 0x1103,
    RET_ERROR_FLASH_PROG                = 0x1104,

    RET_ERROR_FAT_INIT_ERROR            = 0x2000,
    RET_ERROR_FAT_REINIT_ERROR          = 0x2001,
    RET_ERROR_FAT_READ_FILE_ERROR       = 0x2002,
    ERROR_FAT_WRITE_FILE                = 0x2005,
    ERROR_FAT_MOUNT_DISK                = 0x2006,
    ERROR_FAT_LSEEK_FILE                = 0x2007,
    ERROR_FAT_CLOSE_FILE                = 0x2008,

    RET_ERROR_RTOS_TASK                 = 0x5000,
    RET_ERROR_RTOS_TASK_NOT_CREATE      = 0x5010,
    RET_ERROR_RTOS_START_SHEDULER       = 0x5020,
    RET_ERROR_RTOS_QUEUE_CREATE         = 0x5100,
    RET_ERROR_RTOS_QUEUE_SEND           = 0x5110,

    // warnings ================================================================
    RET_WARNING_ADC_SHORT2GND           = 0x8001,
    RET_WARNING_ADC_SHORT2VCC           = 0x8002,

} ;

typedef int RET;

enum MUTEX_LOCK_type {
    MUTEX_UNLOCKED                      = 0x00U,
    MUTEX_LOCKED                        = 0x01U
} ;


enum CMD_enum {
    CMD_NOP                   			= 0,

    CMD_STOP                            = 1,
    CMD_RUN                             = 2,
    CMD_REBOOT                          = 3,

    CMD_GET_ID                          = 4, //get unique IDs
    CMD_GET_VERSION                     = 5, //get firmware\board version

    CMD_FLASH_READ                      = 6, //read FLASH/SPIFLASH/EEPROM
    CMD_FLASH_WRITE                     = 7, //write FLASH/SPIFLASH/EEPROM
    CMD_FLASH_FORMAT                    = 8, //format FLASH/SPIFLASH/EEPROM, sector or all
    CMD_FLASH_ERASE                     = 9,
    CMD_FLASH_FINALIZE                  = 10,

//    CMD_VALUE_READ                      = 9,
//    CMD_VALUE_WRITE                     = 10,
//    CMD_VALUE_RESET                     = 11,

    CMD_GET_DATA_BLOCK                  = 12,
    CMD_GET_LOG_BLOCK                   = 13,

    CMD_PING                            = 20, // ping, respond same
    CMD_RESPOND                         = 21, //common respond for prev commands

    CMD_DISCOVERY                       = 40, /* find dev. in net */
    CMD_DISCOVERY_RESPONSE              = 41, /* respond - text */

    CMD_BOOT_MODE_SET                   = 50,

    CMD_VCOMP_SET                       = 60, /* Virtual communication port */

    CMD_CALIBRATIONS_SET                = 80,
    CMD_CALIBRATE_TO_DEFAULT            = 81,
    CMD_CALIBRATE_ACC                   = 82,
    CMD_CALIBRATE_GIR                   = 83,
    CMD_CALIBRATE_MAG                   = 84,
    CMD_CALIBRATE_SERVO                 = 85,
    CMD_CALIBRATE_SAVE                  = 86,

    CMD_SET_MODE                        = 90 //custom mode set

};


#define VOID void
//typedef void VOID;
typedef uint8_t U8;
//typedef uint8_t u8;
typedef int8_t  S8;
//typedef int8_t  s8;
typedef uint16_t U16;
//typedef uint16_t u16;
typedef int16_t  S16;
//typedef int16_t  s16;
typedef uint32_t U32;
//typedef uint32_t u32;
typedef int32_t  S32;
//typedef int32_t  s32;
#ifndef STM8
typedef uint64_t U64;
//typedef uint64_t u64;
typedef int64_t  S64;
//typedef int64_t  s64;
#else
typedef uint32_t U64;  // todo
typedef int32_t  S64;
#endif
typedef float FLOAT;
typedef double DOUBLE;
typedef int INT;
typedef unsigned int UINT;
typedef unsigned int uint;
typedef char CHAR;
#ifndef WCHAR
//typedef S16 WCHAR;
#endif
#ifndef TCHAR
//typedef int16_t TCHAR;
#endif
#ifndef MCHAR
typedef S16 MCHAR;
#endif

#if defined(STM8)
    typedef U8 FAST_U8;
    typedef S8 FAST_S8;
    typedef U32 FAST_U16;
    typedef S32 FAST_S16;
    //typedef U8 RET;
#else
    typedef U32 FAST_U8;
//    typedef U32 fast_u8;
    typedef S32 FAST_S8;
//    typedef S32 fast_s8;
    typedef U32 FAST_U16;
//    typedef U32 fast_u16;
    typedef S32 FAST_S16;
#endif

#ifndef BOOL
    #if defined(STM8)
        typedef U8 BOOL;
    #else
        typedef INT BOOL;
    #endif
#endif

#define U8_MAX     ((U8)255)
#define S8_MAX     ((S8)127)
#define S8_MIN     ((S8)-128)
#define U16_MAX    ((U16)65535u)
#define S16_MAX    ((S16)32767)
#define S16_MIN    ((S16)-32768)
#define U32_MAX    ((U32)4294967295uL)
#define S32_MAX    ((S32)2147483647)
#define S32_MIN    ((S32)2147483648uL)
//#define S64_MIN    ((S64)−9223372036854775808LL)
#define S64_MAX    ((S64)+9223372036854775808LL)
#define U64_MAX    ((U64)+18446744073709551615)

#ifndef NULL
  #define NULL                          ((void *)0)
#endif

#ifndef LOW
    #define LOW 0
#endif
#ifndef HIGH
    #define HIGH 1
#endif

#ifndef FALSE
#ifndef TRUE
typedef enum {
    FALSE = 0,
    TRUE  = 1,
    MAYBE = 2
} BOOL_t;
#endif
#endif


struct REGADR8VAL8 {
    U8 reg_addr;
    U8 value;
};

struct REGADR8VAL16 {
    U8 reg_addr;
    U16 value;
};

struct REGADR16VAL16 {
    U16 reg_addr;
    U16 value;
};

#define mkstr(s) #s

//#define UNUSED(X) (void)X
//#define _UNUSED(x) ((void)(x))

#define CODE                            //ICACHE_FLASH_ATTR
//#define ROM                                //ICACHE_RODATA_ATTR

#if defined(STM32)
#ifndef WEAK
#define WEAK                            __attribute__((weak)) //__weak
#endif

#define __ALIGN_START

#endif

//#define    INLINE                          inline
#ifndef INLINE
#define INLINE __attribute__((__always_inline__)) inline
#endif

#define ALIGN_START
#define ALIGN_END                        __attribute__((aligned (4)))
//#define MEM_ALIGN(x) (((x) + sizeof(ALIGN_TYPE) - 1) & ~(sizeof(ALIGN_TYPE)-1))

#if defined ( __ICCARM__ ) /* IAR Compiler */
#define PACK_START                        #pragma pack(push,1)
#define PACK_END                        #pragma pack(pop)
#elif defined   (__CC_ARM)      /* ARM Compiler */

#elif defined ( __GNUC__ ) && !defined (__CC_ARM) /* GNU Compiler */
#define PACK_START                        __attribute__((packed,aligned(1)))
#define PACK_END
#define DATA_PACK                        __attribute__((packed))
#else
//#error "Unknown compiler!"
#endif
/* Macro to get variable aligned on 4-bytes, for __ICCARM__ the directive "#pragma data_alignment=4" must be used instead */

#ifndef PSTR
#define PSTR                             (const char*)
#endif


#define CHAR_END                    0x00 //'\0'
#define CHAR_CR                     0x0D //'\r'
#define CHAR_LR                     0x0A //'\n'

#define ASCII_NUM_0                     ((U8)48)

#define _PI 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214
#define _2PI (PI*2.0)

#define RAD2DEG                         (180.0f/(FLOAT)_PI) //умножаем на RAD2DEG — мы переводим радианы в градусы
#define DEG2RAD                         ((FLOAT)_PI/180.0f) //mul to DEG2RAD - градусы в радианы

#define  _number_is_2_POW_K(x)          ((!((x)&((x)-1)))&&((x)>1))  // x is pow(2, k), k=1,2, ...

#define _clear_bit(reg, bit)               reg &= (~(1<<(bit)))

#define _set_bit(reg, bit)              reg |= (1<<(bit))

#define _bit_is_clear(reg, bit)            ((reg & (1<<(bit))) == 0)

#define _bit_is_set(reg, bit)           ((reg & (1<<(bit))) != 0)

#define _inv_bit(reg, bit)              reg ^= (1<<(bit))

#define _sign(x)                        ((x)<0?-1:1)

#ifndef _abs
#define _abs(x)                         (((x)>=0)?(x):(-x))
#endif

#define _roundf(x)                      floor(x + 0.5f)
#define _round(x)                       floor(x + 0.5)

#ifndef _min
#define _min(a,b)                       ((a)>(b))?(a):(b)
#endif

#ifndef _max
#define _max(a,b)                       ((a)<(b))?(b):(a)
#endif

#define _swap(a, b)                     do { (a)=(a)^(b); (b)=(a)^(b); (a)=(a)^(b); } while(0)

#define _constrain(amt,low,high)        ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))  //clamp


// The gravity acceleration in New York City - 9.802 in m/s^2
// Значение свободного падения (гравитации) в м/с^2
#define GRAVITY                          9.81


// Determine Little-Endian or Big-Endian =======================================
#define CURRENT_BYTE_ORDER               (*(int *)"\x01\x02\x03\x04")
#define LITTLE_ENDIAN_BYTE_ORDER         0x04030201
#define BIG_ENDIAN_BYTE_ORDER            0x01020304
#define PDP_ENDIAN_BYTE_ORDER            0x02010403

#define IS_LITTLE_ENDIAN                 (CURRENT_BYTE_ORDER == LITTLE_ENDIAN_BYTE_ORDER)
#define IS_BIG_ENDIAN                    (CURRENT_BYTE_ORDER == BIG_ENDIAN_BYTE_ORDER)
#define IS_PDP_ENDIAN                    (CURRENT_BYTE_ORDER == PDP_ENDIAN_BYTE_ORDER)


/*
 * Big Endian conversion
 */
//  LE РІ BE, 16
#ifndef htons
#define htons(a) ((((U16)(a) >> 8) & 0x00FF) | (((U16)(a) << 8) & 0xFF00))
#endif

//  BE РІ LE, 16
#ifndef ntohs
#define ntohs(a)            htons(a)
#endif

// LE РІ BE, 32
#ifndef htonl
#define htonl(a)            ((((U32)(a) >> 24) & 0x000000FF) |\
                             (((U32)(a) >>  8) & 0x0000FF00) |\
                             (((U32)(a) <<  8) & 0x00FF0000) |\
                             (((U32)(a) << 24) & 0xFF000000))
#endif

//  BE РІ LE, 32
#ifndef ntohl
#define ntohl(a)            htonl(a)
#endif

/*
#define inet_addr(a,b,c,d)    (((U32)d << 24) |\
                             ((U32)c << 16) |\
                             ((U32)b <<  8) |\
                              (U32)a )
*/


#define REVERSE_ENDIENESS_UINT16(uint16Var) \
       ( ( ((LEP_UINT16)LOW_BYTE(uint16Var))<<8) + (LEP_UINT16)HIGH_BYTE(uint16Var))

#define REVERSE_ENDIENESS_UINT32(uint32Var) \
       ( ((LEP_UINT32)REVERSE_ENDIENESS_UINT16(LOW_WORD(uint32Var)) << 16) + \
         (LEP_UINT32)REVERSE_ENDIENESS_UINT16(HIGH_WORD(uint32Var) ) )

#define REVERSE_NIBBLE_UINT8(uint8Var) \
       ( ( ((LEP_UINT8)LOW_NIBBLE(uint8Var))<<4) + (LEP_UINT8)HIGH_NIBBLE(uint8Var))

#define REVERSE_BYTEORDER_UINT32(uint32Var) \
       ( (((LEP_UINT32)LOW_BYTE(uint32Var))<<24) + (((LEP_UINT32)HIGH_BYTE(uint32Var))<<16) + \
         (((LEP_UINT32)LOW_BYTE(HIGH_WORD(uint32Var)))<<8) + (LEP_UINT32)HIGH_BYTE(HIGH_WORD(uint32Var)) )

#define WORD_SWAP_16(uint32Var)  \
        ( ((LEP_UINT16)LOW_WORD(uint32Var) << 16) + ((LEP_UINT16)HIGH_WORD(uint32Var)) )


enum EDITABLE {
    NO_EDIT = 0,
    EDIT_BOOL = 1,
    EDIT_U8 = 2,
    EDIT_U16 = 3,
    EDIT_U32 = 4,
    EDIT_S8 = 5,
    EDIT_S16 = 6,
    EDIT_S32 = 7,
    EDIT_FLOAT = 8,
    EDIT_DOUBLE = 9,
    EDIT_STRING = 10,
    EDIT_CHAR = 11,
};

// defined this on IDE config!
//#define TEST_TYPE    UNIT_TEST_FULL
//#define TEST_TYPE    UNIT_TEST

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is true, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is false, it returns no value.
  * @retval None
  */
#ifdef UNIT_TEST_FULL
    #define test_param(expr)     ((expr) ? error_handler_file_line( (const char *)__FILE__, __LINE__ ) : (void)0U )
    void error_handler_file_line( const char *file, int line );

#else
#ifdef UNIT_TEST

#ifdef STM8
    #define test_param(expr)     if(expr){ __trap(); while(1){}; }
#else
    #define test_param(expr)     if(expr){ while(1){}; } //__BKPT( 1 );
#endif

#else
    #define test_param(expr)     ((void)0U)
#endif
#endif

//#define __assert(a)              test_param(a)

#ifdef    __cplusplus
extern "C" {
#endif

#ifdef    __cplusplus
}
#endif

#endif /** COMMON_H */
