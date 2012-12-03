/***************************************************************************/

/*
 *	nettel.c -- startup code support for the NETtel boards
 *
 *	Copyright (C) 2009, Greg Ungerer (gerg@snapgear.com)
 */

/***************************************************************************/

#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include <asm/nettel.h>

#define	COLDFIRE_NE2000_FUNCS
#include <asm/mcfne.h>

/***************************************************************************/

static struct resource nettel_ne2k_resources[] = {
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

/***************************************************************************/

static void __init nettel_ne2k_init(void)
{
	u32 pitr;

	/* We want IRQ3 with high to low edge detection */
	pitr = readl(MCF_MBAR + MCFSIM_PITR);
	writel(pitr | 0x20000000, MCF_MBAR + MCFSIM_PITR);
}

/***************************************************************************/

static int __init init_nettel(void)
{
	nettel_ne2k_init();
	platform_device_register_simple("ne", -1, nettel_ne2k_resources,
		ARRAY_SIZE(nettel_ne2k_resources));
	return 0;
}

arch_initcall(init_nettel);

/***************************************************************************/
