/*
 * asm/arch/uncompress.c:
 *         Optional routines to aid in debugging the decompression phase
 *         of kernel boot.
 * copyright:
 *         (C) 2001 RidgeRun, Inc. (http://www.ridgerun.com)
 * author: Gordon McNutt <gmcnutt@ridgerun.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 * WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 * USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <mach/hardware.h>

#define VPint   *(volatile unsigned int *)

#ifndef CSR_WRITE
#   define CSR_WRITE(addr,data) (VPint(addr) = (data))
#endif

#ifndef CSR_READ
#   define CSR_READ(addr)       (VPint(addr))
#endif

/** Console UART Port */
#define DEBUG_CONSOLE   (0)

#if (DEBUG_CONSOLE == 0)
	#define DEBUG_TX_BUFF_BASE	REG_UART0_TXB
	#define DEBUG_RX_BUFF_BASE      REG_UART0_RXB
	#define DEBUG_UARTLCON_BASE     REG_UART0_LCON
	#define DEBUG_UARTCONT_BASE     REG_UART0_CTRL
	#define DEBUG_UARTBRD_BASE      REG_UART0_BAUD_DIV
	#define DEBUG_CHK_STAT_BASE     REG_UART0_STAT
#else /* DEBUG_CONSOLE == 1 */
	#define DEBUG_TX_BUFF_BASE      REG_UART1_TXB
	#define DEBUG_RX_BUFF_BASE      REG_UART1_RXB
	#define DEBUG_UARTLCON_BASE     REG_UART1_LCON
	#define DEBUG_UARTCONT_BASE     REG_UART1_CTRL
	#define DEBUG_UARTBRD_BASE      REG_UART1_BAUD_DIV
	#define DEBUG_CHK_STAT_BASE     REG_UART1_STAT
#endif

#define DEBUG_ULCON_REG_VAL     (0x3)
#define DEBUG_UCON_REG_VAL      (0x9)
#define DEBUG_UBRDIV_REG_VAL    (0x500)
#define DEBUG_RX_CHECK_BIT      (0X20)
#define DEBUG_TX_CAN_CHECK_BIT  (0X40)
#define DEBUG_TX_DONE_CHECK_BIT (0X80)

/*
 * Setup console UART as 19200 bps.
 */
static inline void arch_decomp_setup(void)
{
	CSR_WRITE(DEBUG_UARTLCON_BASE, DEBUG_ULCON_REG_VAL);
	CSR_WRITE(DEBUG_UARTCONT_BASE, DEBUG_UCON_REG_VAL);
	CSR_WRITE(DEBUG_UARTBRD_BASE,  DEBUG_UBRDIV_REG_VAL);
}

static inline void putc(char c)
{
	CSR_WRITE(DEBUG_TX_BUFF_BASE, c);
	while(!(CSR_READ(DEBUG_CHK_STAT_BASE) & DEBUG_TX_DONE_CHECK_BIT));
}

static inline void flush(void)
{
}

