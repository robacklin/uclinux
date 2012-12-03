/*
 * arch/arm/mach-kirkwood/utm300-setup.c
 *
 * McAfee UTM300 Setup
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

static struct mtd_partition utm300_nand_parts[] = {
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

static unsigned int utm300_mpp_config[] __initdata = {
	MPP0_NF_IO2,
	MPP1_NF_IO3,
	MPP2_NF_IO4,
	MPP3_NF_IO5,
	MPP4_NF_IO6,
	MPP5_NF_IO7,
	MPP6_SYSRST_OUTn,
	MPP7_GPO,
	MPP8_UART0_RTS,
	MPP9_UART0_CTS,
	MPP10_UART0_TXD,
	MPP11_UART0_RXD,
	MPP12_GPO,
	MPP13_GPIO,
	MPP14_MII0_COL,
	MPP15_GPIO,
	MPP16_MII0_CRS,
	MPP17_GPIO,
	MPP18_NF_IO0,
	MPP19_NF_IO1,
	MPP35_MII0_RXERR,
	MPP36_GPIO,
	MPP37_GPIO,
	MPP38_GPIO,
	MPP39_GPIO,
	MPP40_GPIO,
	MPP41_GPIO,
	MPP42_GPIO,
	MPP43_GPIO,
	MPP44_GPIO,
	0
};

static struct mv643xx_eth_platform_data utm300_ge00_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(5),
	.speed		= SPEED_100,
	.duplex		= DUPLEX_FULL,
};

static void utm300_usboc_handler(unsigned long data)
{
	static int count_down = 0;
	static DEFINE_TIMER(utm300_usboc_timer, utm300_usboc_handler, 0, 0);

	/*
	 * This routine is called every 20 millseconds to
	 * check the state of USB overcurrent.
	 *
	 * If it's high we turn it off for 5 seconds and try again.
	 */
	if (count_down > 0) {
		count_down--;
		if (count_down == 0) {
			gpio_set_value(15, 1); /* turn on USB */
			printk(KERN_ERR "utm300: Re-enabling USB power after timeout\n");
		}
	} else if (gpio_get_value(17) == 0) {
		gpio_set_value(15, 0); /* turn off USB */
		count_down = 50;
		printk(KERN_ERR "utm300: USB disabled due to OC\n");
	}

	mod_timer(&utm300_usboc_timer, jiffies + msecs_to_jiffies(20));
}

static void __init utm300_gpio_output(int pin, char *name, int val)
{
	if (gpio_request(pin, name) != 0 || gpio_direction_output(pin, val) != 0)
		printk(KERN_ERR "utm300: can't set up output GPIO %d (%s)\n",
				pin, name);
}

static void __init utm300_gpio_input(int pin, char *name)
{
	if (gpio_request(pin, name) != 0 || gpio_direction_input(pin) != 0)
		printk(KERN_ERR "utm300: can't set up input GPIO %d (%s)\n", pin, name);
}

static void __init utm300_init(void)
{
	/*
	 * Basic setup. Needs to be called early.
	 */
	kirkwood_init();
	kirkwood_mpp_conf(utm300_mpp_config);
	utm300_gpio_output(7, "SYSCON", 1);
	kirkwood_uart0_init();

	utm300_gpio_output(38, "D2 - H/B", 0);
	utm300_gpio_output(39, "D3 - ETH A", 0);
	utm300_gpio_output(40, "D4 - ETH B", 0);
	utm300_gpio_output(41, "D5 - HA", 0);
	utm300_gpio_output(42, "D6 - Serial", 0);
	utm300_gpio_output(43, "D7 - Online", 0);
	utm300_gpio_output(44, "D8 - VPN", 0);
	utm300_gpio_output(12, "DTR", 1);
	utm300_gpio_input (13, "DCD");
	utm300_gpio_input (37, "Erase");

	/*
	 * USB enable/OC checking
	 */
	utm300_gpio_output(15, "USB Enable", 1);
	utm300_gpio_input (17, "/USB OC");
	mdelay(20);
	utm300_usboc_handler(0); /* start the timer, do first check */
	utm300_gpio_output(36, "Watchdog Service", 0);

	/* enable PCI in CPU control reg */
	writel(readl(KIRKWOOD_REGS_VIRT_BASE | 0x20104) | 0x1,
			KIRKWOOD_REGS_VIRT_BASE | 0x20104);
	/* give time to PCI to settle and be ready or we miss devices */
	mdelay(200);

	kirkwood_ehci_init();

	kirkwood_ge00_init(&utm300_ge00_data);

	/* turn on PHY polling for port 0 */
	writel(readl(KIRKWOOD_REGS_VIRT_BASE | 0x720b0) | 0x2,
			KIRKWOOD_REGS_VIRT_BASE | 0x720b0);
	/* turn on PHY polling for port 1 */
	writel(readl(KIRKWOOD_REGS_VIRT_BASE | 0x760b0) | 0x2,
			KIRKWOOD_REGS_VIRT_BASE | 0x760b0);

	kirkwood_nand_init(ARRAY_AND_SIZE(utm300_nand_parts), 25);
}

static int __init utm300_pci_init(void)
{
	if (machine_is_utm300())
		kirkwood_pcie_init(KW_PCIE0);
	return 0;
}
subsys_initcall(utm300_pci_init);

MACHINE_START(UTM300, "McAfee UTM300")
	/* Maintainer: Greg Ungerer  <gerg@snapgear.com> */
	.boot_params  = 0x00000100,
	.init_machine = utm300_init,
	.map_io       = kirkwood_map_io,
	.init_irq     = kirkwood_init_irq,
	.timer        = &kirkwood_timer,
MACHINE_END
