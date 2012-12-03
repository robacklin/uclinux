/***************************************************************************/

/*
 *	linux/arch/m68knommu/platform/5307/config.c
 *
 *	Copyright (C) 1999-2002, Greg Ungerer (gerg@snapgear.com)
 *	Copyright (C) 2000, Lineo (www.lineo.com)
 */

/***************************************************************************/

#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <asm/machdep.h>
#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include <asm/mcfwdebug.h>

#ifdef CONFIG_NE2000
#define	COLDFIRE_NE2000_FUNCS
#include <asm/mcfne.h>
#endif

/***************************************************************************/

/*
 *	Some platforms need software versions of the GPIO data registers.
 */
unsigned short ppdata;
unsigned char ledbank = 0xff;

/***************************************************************************/

#ifdef CONFIG_NE2000
static struct resource m5307_ne2k_resources[] = {
	{
		.start		= NE2000_ADDR,
		.end		= NE2000_ADDR + 0x10000,
		.flags		= IORESOURCE_IO,
	},
	{
		.start		= NE2000_IRQ_VECTOR,
		.end		= NE2000_IRQ_VECTOR,
		.flags		= IORESOURCE_IRQ,
	},
};
#endif /* CONFIG NE2000 */

/***************************************************************************/

void __init config_BSP(char *commandp, int size)
{
#if defined(CONFIG_NETtel) || \
    defined(CONFIG_SECUREEDGEMP3) || defined(CONFIG_CLEOPATRA)
	/* Copy command line from FLASH to local buffer... */
	memcpy(commandp, (char *) 0xf0004000, size);
	commandp[size-1] = 0;
#endif

	mach_sched_init = hw_timer_init;

	/* Only support the external interrupts on their primary level */
	mcf_mapirq2imr(25, MCFINTC_EINT1);
	mcf_mapirq2imr(27, MCFINTC_EINT3);
	mcf_mapirq2imr(29, MCFINTC_EINT5);
	mcf_mapirq2imr(31, MCFINTC_EINT7);

#ifdef CONFIG_BDM_DISABLE
	/*
	 * Disable the BDM clocking.  This also turns off most of the rest of
	 * the BDM device.  This is good for EMC reasons. This option is not
	 * incompatible with the memory protection option.
	 */
	wdebug(MCFDEBUG_CSR, MCFDEBUG_CSR_PSTCLK);
#endif
}

/***************************************************************************/

static int __init init_BSP(void)
{
#ifdef CONFIG_NE2000
	platform_device_register_simple("ne", -1, m5307_ne2k_resources,
		ARRAY_SIZE(m5307_ne2k_resources));
#endif
	return 0;
}

arch_initcall(init_BSP);

/***************************************************************************/
