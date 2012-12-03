/*
 * arch/arm/mach-kirkwood/utm400-setup.c
 *
 * McAfee UTM400 Setup
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/ata_platform.h>
#include <linux/mv643xx_eth.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/io.h>
#include <mach/kirkwood.h>
#include <plat/orion_nand.h>
#include "common.h"
#include "mpp.h"

static struct mtd_partition utm400_nand_parts[] = {
	{
		.name = "u-boot",
		.offset = 0,
		.size = SZ_1M
	}, {
		.name = "bootarg",
		.offset = MTDPART_OFS_NXTBLK,
		.size = SZ_1M
	}, {
		.name = "logd",
		.offset = MTDPART_OFS_NXTBLK,
		.size = SZ_1M
	}, {
		.name = "config",
		.offset = MTDPART_OFS_NXTBLK,
		.size = SZ_8M
	}, {
		.name = "image",
		.offset = MTDPART_OFS_NXTBLK,
		.size = SZ_32M
	}, {
		.name = "linux",
		.offset = MTDPART_OFS_NXTBLK,
		.size = SZ_2M
	}, {
		.name = "spare",
		.offset = MTDPART_OFS_NXTBLK,
		.size = MTDPART_SIZ_FULL
	}, {
		.name = "all",
		.offset = 0,
		.size = MTDPART_SIZ_FULL
	},
};

static struct mv643xx_eth_platform_data utm400_ge00_data = {
	/*
	 * the switch is at 0x02,  but we don't want anyone talking to it
	 * as if it were a PHY, so we use 0x12 instead.
	 */
	.phy_addr	= MV643XX_ETH_PHY_ADDR(0x12),
	.speed		= SPEED_1000,
	.duplex		= DUPLEX_FULL,
};

static struct mv643xx_eth_platform_data utm400_ge01_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(0x13),
	.speed		= SPEED_1000,
	.duplex		= DUPLEX_FULL,
};

static unsigned int utm400_mpp_config[] __initdata = {
	MPP0_NF_IO2,
	MPP1_NF_IO3,
	MPP2_NF_IO4,
	MPP3_NF_IO5,
	MPP4_NF_IO6,
	MPP5_NF_IO7,
	MPP6_SYSRST_OUTn,
	MPP7_GPO,
	MPP8_GPIO,
	MPP9_GPIO,
	MPP10_UART0_TXD,
	MPP11_UART0_RXD,
	MPP12_GPO,
	MPP13_GPIO,
	MPP14_GPIO,
	MPP15_UART0_RTS,
	MPP16_UART0_CTS,
	MPP17_GPIO,
	MPP18_NF_IO0,
	MPP19_NF_IO1,
	MPP20_GE1_TXD0,
	MPP21_GE1_TXD1,
	MPP22_GE1_TXD2,
	MPP23_GE1_TXD3,
	MPP24_GE1_RXD0,
	MPP25_GE1_RXD1,
	MPP26_GE1_RXD2,
	MPP27_GE1_RXD3,
	MPP28_GPIO,
	MPP29_GPIO,
	MPP30_GE1_RXCTL,
	MPP31_GE1_RXCLK,
	MPP32_GE1_TCLKOUT,
	MPP33_GE1_TXCTL,
	MPP34_GPIO,
	MPP35_GPIO,
	MPP36_GPIO,
	MPP37_GPIO,
	MPP38_GPIO,
	MPP39_GPIO,
	MPP40_GPIO,
	MPP41_GPIO,
	MPP42_GPIO,
	MPP43_GPIO,
	MPP44_GPIO,
	MPP45_GPIO,
	MPP46_GPIO,
	MPP47_GPIO,
	MPP48_GPIO,
	MPP49_GPIO,
	0
};

#ifdef CONFIG_SATA_MV
/* if you enable the driver assume you want it to work */
static struct mv_sata_platform_data utm400_sata_data = {
	.n_ports	= 1,
};
#endif


static void utm400_usboc_handler(unsigned long data)
{
	static int count_down = 0;
	static DEFINE_TIMER(utm400_usboc_timer, utm400_usboc_handler, 0, 0);

	/*
	 * This routine is called every 20 millseconds to
	 * check the state of USB overcurrent.
	 *
	 * If it's high we turn it off for 5 seconds and try again.
	 */
	if (count_down > 0) {
		count_down--;
		if (count_down == 0) {
			gpio_set_value(42, 1); /* turn on USB */
			printk(KERN_ERR "utm400: Re-enabling USB power after timeout\n");
		}
	} else if (gpio_get_value(43) == 0) {
		gpio_set_value(42, 0); /* turn off USB */
		count_down = 50;
		printk(KERN_ERR "utm400: USB disabled due to OC\n");
	}

	mod_timer(&utm400_usboc_timer, jiffies + msecs_to_jiffies(20));
}

static void __init utm400_gpio_output(int pin, char *name, int val)
{
	if (gpio_request(pin, name) != 0 ||
			gpio_direction_output(pin, val) != 0)
		printk(KERN_ERR "utm400: can't set up output GPIO %d (%s)\n",
				pin, name);
}

static void __init utm400_gpio_input(int pin, char *name)
{
	if (gpio_request(pin, name) != 0 || gpio_direction_input(pin) != 0)
		printk(KERN_ERR "utm400: can't set up input GPIO %d (%s)\n", pin, name);
}

static void __init utm400_init(void)
{
	/*
	 * Basic setup. Needs to be called early.
	 */
	kirkwood_init();
	kirkwood_mpp_conf(utm400_mpp_config);
	kirkwood_uart0_init();

	utm400_gpio_output(12, "D2 - Online", 0);
	utm400_gpio_output(13, "D3 - ETH", 0);
	utm400_gpio_output(28, "D4 - USB", 0);
	utm400_gpio_output(29, "D5 - Serial", 0);
	utm400_gpio_output(36, "D6 - HA", 0);
	utm400_gpio_output(37, "D7 - Online", 0);
	utm400_gpio_output(38, "D8 - VPN", 0);
	utm400_gpio_output(39, "DTR", 1);
	utm400_gpio_input (40, "DCD");
	utm400_gpio_input (41, "Erase");

	/*
	 * USB enable/OC checking
	 */
	utm400_gpio_output(42, "USB Enable", 1);
	utm400_gpio_input (43, "/USB OC");
	mdelay(20);
	utm400_usboc_handler(0); /* start the timer, do first check */

	utm400_gpio_output(44, "Watchdog Service", 0);
	utm400_gpio_input (45, "NF_R/B");
	utm400_gpio_input (46, "/SW_INT");
    /*
	 * actually do a PCIE reset while we are here
	 */
	utm400_gpio_output(47, "PCIE Reset", 0);
	mdelay(20);
	gpio_set_value(47, 1);
	/* enable PCI in CPU control reg */
	writel(readl(KIRKWOOD_REGS_VIRT_BASE | 0x20104) | 0x1,
			KIRKWOOD_REGS_VIRT_BASE | 0x20104);
	/* give time to PCI to settle and be ready or we miss devices */
	mdelay(200);

	kirkwood_ehci_init();

#ifdef CONFIG_SATA_MV
	/* if you enable the driver assume you want it to work */
	kirkwood_sata_init(&utm400_sata_data);
#endif

	kirkwood_ge00_init(&utm400_ge00_data);
	kirkwood_ge01_init(&utm400_ge01_data);

	/* turn on PHY polling for port 0 */
	writel(readl(KIRKWOOD_REGS_VIRT_BASE | 0x720b0) | 0x2,
			KIRKWOOD_REGS_VIRT_BASE | 0x720b0);
	/* turn on PHY polling for port 1 */
	writel(readl(KIRKWOOD_REGS_VIRT_BASE | 0x760b0) | 0x2,
			KIRKWOOD_REGS_VIRT_BASE | 0x760b0);

	kirkwood_nand_init(ARRAY_AND_SIZE(utm400_nand_parts), 25);
}

static int __init utm400_pci_init(void)
{
	if (machine_is_utm400())
		kirkwood_pcie_init(KW_PCIE0);
	return 0;
}
subsys_initcall(utm400_pci_init);

MACHINE_START(UTM400, "McAfee UTM400")
	/* Maintainer: David McCullough <david_mccullough@mcafee.com> */
	.atag_offset  = 0x00000100,
	.init_machine = utm400_init,
	.map_io       = kirkwood_map_io,
	.init_irq     = kirkwood_init_irq,
	.timer        = &kirkwood_timer,
MACHINE_END
