/****************************************************************************/

/*
 *	uclinux.c -- generic memory mapped MTD driver for uclinux
 *
 *	(C) Copyright 2002, Greg Ungerer (gerg@snapgear.com)
 */

/****************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/root_dev.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>

/****************************************************************************/

#if defined(CONFIG_MTD_UCLINUX_EBSS)
	#define MAP_TYPE	"map_ram"
	#define MAP_NAME	"RAM"
	#define  CONFIG_MTD_UCLINUX_ADDRESS &_ebss
	extern char _ebss;
#elif defined(CONFIG_MTD_UCLINUX_RAM)
	#define MAP_TYPE	"map_ram"
	#define MAP_NAME	"RAM"
#elif defined(CONFIG_MTD_UCLINUX_ROM)
	#define MAP_TYPE	"map_rom"
	#define MAP_NAME	"ROM"
#else
	#error "Unknown uClinux map type"
#endif

/****************************************************************************/

struct map_info uclinux_map = {
	.name = MAP_NAME,
};

static struct mtd_info *uclinux_mtdinfo;

/****************************************************************************/

static struct mtd_partition uclinux_fs[] = {
	{ .name = "ROMfs" }
};

#define	NUM_PARTITIONS	ARRAY_SIZE(uclinux_fs)

/****************************************************************************/

static int uclinux_point(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, void **virt, resource_size_t *phys)
{
	struct map_info *map = mtd->priv;
	*virt = map->virt + from;
	if (phys)
		*phys = map->phys + from;
	*retlen = len;
	return(0);
}

/****************************************************************************/

static int __init uclinux_mtd_init(void)
{
	struct mtd_info *mtd;
	struct map_info *mapp;
	unsigned long addr = (unsigned long) CONFIG_MTD_UCLINUX_ADDRESS;

	mapp = &uclinux_map;
	mapp->phys = addr;
	if (!mapp->size)
		mapp->size = PAGE_ALIGN(ntohl(*((unsigned long *)(addr + 8))));
	mapp->bankwidth = 4;

	printk("uclinux[mtd]: RAM probe address=0x%x size=0x%x\n",
	       	(int) mapp->phys, (int) mapp->size);

	mapp->virt = phys_to_virt(mapp->phys);

	if (mapp->virt == 0) {
		printk("uclinux[mtd]: ioremap_nocache() failed\n");
		return(-EIO);
	}

	simple_map_init(mapp);

	mtd = do_map_probe(MAP_TYPE, mapp);
	if (!mtd) {
		printk("uclinux[mtd]: failed to find a mapping?\n");
		return(-ENXIO);
	}

	mtd->owner = THIS_MODULE;
	mtd->_point = uclinux_point;
	mtd->priv = mapp;

	printk("uclinux[mtd]: set %s to be root filesystem\n",
	     	uclinux_fs[0].name);
	ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, 0);

	uclinux_mtdinfo = mtd;
	mtd_device_register(mtd, uclinux_fs, NUM_PARTITIONS);

	return(0);
}

/****************************************************************************/

static void __exit uclinux_mtd_cleanup(void)
{
	if (uclinux_mtdinfo) {
		mtd_device_unregister(uclinux_mtdinfo);
		map_destroy(uclinux_mtdinfo);
		uclinux_mtdinfo = NULL;
	}
	if (uclinux_map.virt)
		uclinux_map.virt = 0;
}

/****************************************************************************/

module_init(uclinux_mtd_init);
module_exit(uclinux_mtd_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greg Ungerer <gerg@snapgear.com>");
MODULE_DESCRIPTION("Generic RAM based MTD for uClinux");

/****************************************************************************/
