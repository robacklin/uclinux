/*
 * linux/include/asm-arm/arch-lpc22xx/uncompress.h
 *
 * Created Jan 04, 2007 Brandon Fosdick <brandon@teslamotors.com>
 */

#include <mach/lpc28xx.h>	/* For U0LSR	*/

#ifndef LPC28XX_UNCOMPRESSH
#define LPC28XX_UNCOMPRESSH

#define TEMP (1<<6)

static void putc(char c)
{
	while(!(UART_LSR & TEMP)){}
	UART_THR = c;
}

static void flush(void) {}

#define arch_decomp_setup()
#define arch_decomp_wdog()

#endif	//LPC28XX_UNCOMPRESSH
