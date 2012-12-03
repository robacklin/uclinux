/****************************************************************************/

/*
 *	snapshow.c --  collect and display the ADSL showtime status
 *
 *	(C) Copyright 2009,  Greg Ungerer <gerg@snapgear.com>
 */

/****************************************************************************/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/io.h>

/****************************************************************************/

static volatile u8 *showt_cs2;

/****************************************************************************/

static int showt_read_proc(char *buf, char **start, off_t off, int size, int *eof, void *data)
{
	int len, link;

	link = (*showt_cs2 & 0x1) ? 0 : 1;
	len = sprintf(buf, "%d\n", link);
	*eof = 1;
	if (off >= len)
		len = 0;
	*start = buf;
	return len;
}

/****************************************************************************/

static __init int showt_init(void)
{
	printk("SNAPSHOW: SnapGear xDSL SHOWTIME status driver\n");

	/* Configure the CS1 line as 8bit input */
	*IXP4XX_EXP_CS1 = 0xbfff0001;

	showt_cs2 = (volatile u8 *) ioremap(IXP4XX_EXP_BUS_BASE(1), 16);
	if (showt_cs2 == NULL) {
		printk("SHOWTIME: cannot remap io region?\n");
		return -ENODEV;
	}

	if (!create_proc_read_entry("showtime", 0, NULL, showt_read_proc, NULL)) {
		printk("SHOWTIME: failed to register proc entry?\n");
		return -ENOMEM;
	}

	return 0;
}

module_init(showt_init);

/****************************************************************************/
