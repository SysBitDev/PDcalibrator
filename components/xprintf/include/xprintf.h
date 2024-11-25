/*------------------------------------------------------------------------*/
/* Universal string handler for user console interface  (C)ChaN, 2021     */
/*------------------------------------------------------------------------*/

#ifndef XPRINTF_DEF
#define XPRINTF_DEF 20230923

#include "board.h"
#include <string.h>

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef XF_USE_OUTPUT
#define XF_USE_OUTPUT	1	/* 1: Enable output functions */
#endif

#ifndef XF_CRLF
#define	XF_CRLF			0	/* 1: Convert \n ==> \r\n in the output char */
#endif

#ifndef XF_USE_DUMP
#define	XF_USE_DUMP		0	/* 1: Enable put_dump function */
#endif

#ifndef XF_USE_LLI
#define	XF_USE_LLI		0	/* 1: Enable long long integer in size prefix ll */
#endif

#ifndef XF_USE_FP
#define	XF_USE_FP		1	/* 1: Enable support for floating point in type e and f */
#endif

#ifndef XF_DPC
#define XF_DPC			'.'	/* Decimal separator for floating point */
#endif

#ifndef XF_USE_INPUT
#define XF_USE_INPUT	0	/* 1: Enable input functions */
#endif

#ifndef XF_INPUT_ECHO
#define	XF_INPUT_ECHO	0	/* 1: Echo back input chars in xgets function */
#endif

#if defined(__GNUC__) && __GNUC__ >= 10
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

#if XF_USE_OUTPUT
#define xdev_out(func) xfunc_output = (void(*)(int))(func)
extern void (*xfunc_output)(int);
void xputc (int chr);
void xfputc (void (*func)(int), int chr);
void xputs (const char* str);
void xfputs (void (*func)(int), const char* str);

#include <stdarg.h>
void xvfprintf( void(*func)(int), const char* fmt, va_list arp );

void xprintf (const char* fmt, ...);
void xsprintf (char* buff, const char* fmt, ...);
void xfprintf (void (*func)(int), const char* fmt, ...);
void put_dump (const void* buff, unsigned long addr, int len, size_t width);
#endif

#if XF_USE_INPUT
#define xdev_in(func) xfunc_input = (int(*)(void))(func)
extern int (*xfunc_input)(void);
int xgets (char* buff, int len);
int xatoi (char** str, long* res);
int xatof (char** str, double* res);
#endif

/* ZAY079 */
/**
 * @brief xsprintf
 * @param buff
 * @param fmt
 * @return - size in bytes including 0 terminal
 */
INT xsprintfs( char* buff, const char *fmt, ... );



#ifdef	__cplusplus
}
#endif

#endif //XPRINTF_DEF
