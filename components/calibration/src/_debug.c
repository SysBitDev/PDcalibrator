#include "_debug.h"
#include "board.h"


#if( defined( UNIT_TEST ) )

void CODE debug_cnt_init( void )
{
#if ( defined(STM32F1) || defined(STM32F2) || defined(STM32F3) || defined(STM32F4) || defined(STM32H7) || defined(STM32L1) || defined(STM32G4) || defined(STM32L5) )
    SCB_DEMCR   |= CoreDebug_DEMCR_TRCENA_Msk; // init counter
    DWT_CONTROL |= DWT_CTRL_CYCCNTENA_Msk;   // cntr on
#endif
}


#if( defined( DEBUG_SWO ) )
void CODE debug_SWO_init( void )
{
//  SWO
// <h> Debug MCU Configuration
//   <o1.0>    DBG_SLEEP     <i> Debug Sleep Mode
//   <o1.1>    DBG_STOP      <i> Debug Stop Mode
//   <o1.2>    DBG_STANDBY   <i> Debug Standby Mode
//   <o1.5>    TRACE_IOEN    <i> Trace I/O Enable 
//   <o1.6..7> TRACE_MODE    <i> Trace Mode
//             <0=> Asynchronous
//             <1=> Synchronous: TRACEDATA Size 1
//             <2=> Synchronous: TRACEDATA Size 2
//             <3=> Synchronous: TRACEDATA Size 4
//   <o1.8>    DBG_IWDG_STOP <i> Independant Watchdog Stopped when Core is halted
//   <o1.9>    DBG_WWDG_STOP <i> Window Watchdog Stopped when Core is halted
//   <o1.10>   DBG_TIM1_STOP <i> Timer 1 Stopped when Core is halted
//   <o1.11>   DBG_TIM2_STOP <i> Timer 2 Stopped when Core is halted
//   <o1.12>   DBG_TIM3_STOP <i> Timer 3 Stopped when Core is halted
//   <o1.13>   DBG_TIM4_STOP <i> Timer 4 Stopped when Core is halted
//   <o1.14>   DBG_CAN_STOP  <i> CAN Stopped when Core is halted
// </h>
  //_WDWORD(0xE0042004, 0x00000027);  // DBGMCU_CR
    DBGMCU_CR = 0x00000027;
}

#endif


void CODE _debug_init( void )
{
	debug_cnt_init();

#if( defined( DEBUG_SWO ) )
	debug_SWO_init();
#endif

#if( defined( DEBUG_ARM_DIV_BY_ZERO ) )
    SCB->CCR |= 0x18;                // enable div-by-0 and unaligned fault
#endif

#if( defined( DEBUG_ARM_USAGE_BUS_MMU_FAULT ) )
	SCB->SHCSR |= 0x00007000;        // enable Usage Fault, Bus Fault, and MMU Fault
#endif
}

#endif


#if( defined( DEBUG_SWO ) )
//#include <stdio.h>
//struct __FILE { int handle;  }; // Add whatever needed //
//FILE __stdout;
//FILE __stdin;
int fputc_( int c, FILE *stream )
{
    return(ITM_SendChar(c));

    if( DEMCR & TRCENA )
    {
        while (ITM_Port32(0) == 0) {};
        ITM_Port8(0) = c;
    }
    return ( c );
}

#endif


#if( defined(STM32) )
BOOL debug_mode_get( void )
{
    volatile U32 _DHCSR;

    _DHCSR = *(U32*)0xE000EDF0;    //Debug Halting Control and Status Register

    if((_DHCSR & (1<<0)) != 0)
    {
        return TRUE;   //Debug mode enable
    }
    else
    {
        return FALSE;   //Debug mode disable
    }
}
#endif


#if( defined(UNIT_TEST) && defined(STM32) )

void HardFault_Handler( void )
{
	__asm volatile	(
" 		MOVS   R0, #4							\n" /* Determine if processor uses PSP or MSP by checking bit.4 at LR register.		*/
"		MOV    R1, LR							\n"
"		TST    R0, R1							\n"
"		BEQ    _IS_MSP							\n" /* Jump to '_MSP' if processor uses MSP stack.									*/
"_IS_PSP:                                       \n"
"		MRS    R0, PSP							\n" /* Prepare PSP content as parameter to the calling function below.				*/
"		BL	   hard_fault_handler_c      		\n" /* Call 'hard_fault_handler_c' passing PSP content as stackedContextPtr value.	*/
"_IS_MSP:										\n"
"		MRS    R0, MSP							\n" /* Prepare MSP content as parameter to the calling function below.				*/
"		BL	   hard_fault_handler_c		        \n" /* Call 'hard_fault_handler_c' passing MSP content as stackedContextPtr value.	*/
	::	);
}

/*! ============================================================================
* hard fault handler in C,
* with stack frame location as input parameter
* called from HardFault_Handler
*/
//int printf(const char *_Restrict, ...);
void hard_fault_handler_c(unsigned int * stackedContextPtr)
{
    volatile U32 stacked_r0 __attribute__((unused));
    volatile U32 stacked_r1 __attribute__((unused));
    volatile U32 stacked_r2 __attribute__((unused));
    volatile U32 stacked_r3 __attribute__((unused));
    volatile U32 stacked_r12 __attribute__((unused));
    volatile U32 stacked_lr __attribute__((unused));
    volatile U32 stacked_pc __attribute__((unused));
    volatile U32 stacked_psr __attribute__((unused));

    // Bus Fault Address Register
    volatile U32 BFAR __attribute__((unused));
    // Configurable Fault Status Register Consists of MMSR, BFSR and UFSR
    volatile U32 CFSR __attribute__((unused));
    // Hard Fault Status Register
    volatile U32 HFSR __attribute__((unused));
    // Debug Fault Status Register
    volatile U32 DFSR __attribute__((unused));
    // Auxiliary Fault Status Register
    volatile U32 AFSR __attribute__((unused));
    // MemManage Fault Address Register
    volatile U32 MMAR __attribute__((unused));

    volatile U32 SCB_SHCSR __attribute__((unused));

    stacked_r0  = stackedContextPtr[0];
    stacked_r1  = stackedContextPtr[1];
    stacked_r2  = stackedContextPtr[2];
    stacked_r3  = stackedContextPtr[3];
    stacked_r12 = stackedContextPtr[4];
    stacked_lr  = stackedContextPtr[5];
    stacked_pc  = stackedContextPtr[6];
    stacked_psr = stackedContextPtr[7];

    // Configurable Fault Status Register
    // Consists of MMSR, BFSR and UFSR
    CFSR = (*((volatile U32 *)(0xE000ED28))) ;

    // Hard Fault Status Register
    HFSR = (*((volatile U32 *)(0xE000ED2C))) ;

    // Debug Fault Status Register
    DFSR = (*((volatile U32 *)(0xE000ED30))) ;

    // Auxiliary Fault Status Register
    AFSR = (*((volatile U32 *)(0xE000ED3C))) ;

    // Read the Fault Address Registers. These may not contain valid values.
    // Check BFARVALID/MMARVALID to see if they are valid values
    // MemManage Fault Address Register
    MMAR = (*((volatile U32 *)(0xE000ED34))) ;
    // Bus Fault Address Register
    BFAR = (*((volatile U32 *)(0xE000ED38))) ;

    SCB_SHCSR = SCB->SHCSR;

    /*
    printf("\n\n[GAME OVER]\n");
    printf("R0 = 0x%008X\n", stacked_r0);
    printf("R1 = 0x%008X\n", stacked_r1);
    printf("R2 = 0x%008X\n", stacked_r2);
    printf("R3 = 0x%008X\n", stacked_r3);
    printf("R12 = 0x%08X\n", stacked_r12);
    printf("LR [R14] = 0x%08X  subroutine call return address\n", stacked_lr);
    printf("PC [R15] = 0x%08X  program counter\n", stacked_pc );
    printf("PSR = 0x%08X\n", stacked_psr);
    printf("BFAR = 0x%08X\n", (*((volatile U32 *)(0xE000ED38))));
    printf("CFSR = 0x%08X\n", (*((volatile U32 *)(0xE000ED28))));
    printf("HFSR = 0x%08X\n", (*((volatile U32 *)(0xE000ED2C))));
    printf("DFSR = 0x%08X\n", (*((volatile U32 *)(0xE000ED30))));
    printf("AFSR = 0x%08X\n", (*((volatile U32 *)(0xE000ED3C))));
    printf("SCB_SHCSR = 0x%08x\n", SCB->SHCSR);
    */
    asm("BKPT #1");
    while(1) {};
}

#else

//void HardFault_Handler( void )
//{
//    while(1){};
//}

#endif //#ifdef UNIT_TEST


//#define MEM_CHECK 1
#if (1 == MEM_CHECK)
static const char hexchars[] = "0123456789ABCDEF";

static char highhex(int  x)
{
   return hexchars[(x >> 4) & 0xF];
}

static char lowhex(int  x)
{
    return hexchars[x & 0xF];
}
/* convert the memory, pointed to by mem into hex, placing result in buf */
 /* return a pointer to the last char put in buf (null) */
static char *mem2hex (const char *mem, char *buf, int count)
{
    int i;
    int ch;
    for (i = 0; i < count; i++)
    {
        ch = *mem++;
        *buf++ = highhex (ch);
        *buf++ = lowhex (ch);
    }
    *buf = 0;
    return (buf);
}
#include "conv.h"
// logf already defined
void _logf(const char *pStr, U32 value) {//"EEPROM address: %d", address);
    char str[32];
    //zspintf (str, "%u", value);
    // concat...
    //out to log file...
    
}
#endif

#if __DEBUG
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void vprint( const char *fmt, va_list argp )
{
    char string[200];
    if( 0 < vsprintf( string, fmt, argp ) ) // build string
    {
    	debug_put( string );
    }
}

void debug_printf( const char *fmt, ... ) // custom printf() function
{
    va_list argp;
    va_start( argp, fmt );
    vprint( fmt, argp );
    va_end( argp );
}


void debug_test( void )
{
/*
	RESET_CORE_COUNT;
	dfA = 0.5;
	dfB = 0.6;
	tic = GET_CORE_COUNT;

	RESET_CORE_COUNT;
	dfC = cos( dfA );
	tic = GET_CORE_COUNT;

	RESET_CORE_COUNT;
	dfC = sqrt( 2.566 );
	tic = GET_CORE_COUNT;


	int f;
	RESET_CORE_COUNT;
	dfC = modf( -10.0002334440003000806, &dfA );
	tic = GET_CORE_COUNT;
*/

}

#endif

