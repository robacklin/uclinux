/*
 * can_read - can4linux CAN driver module
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (c) 2001 port GmbH Halle/Saale
 *------------------------------------------------------------------
 *
 * modification history
 * --------------------
 * Revision 1.1  2005/03/15 12:29:16  vvorobyov
 * CAN support added 2.6 kernel.
 *
 * Revision 1.1.1.2  2003/08/29 01:04:37  davidm
 * Import of uClinux-2.4.22-uc0
 *
 * Revision 1.2  2003/08/28 00:38:31  gerg
 * I hope my patch doesn't come to late for the next uClinux distribution.
 * The new patch is against the latest CVS uClinux-2.4.x/drivers/char. The
 * FlexCAN driver is working but still needs some work. Phil Wilshire is
 * supporting me and we expect to have a complete driver in some weeks.
 *
 * commit text: added support for ColdFire FlexCAN
 *
 * Patch submitted by Heinz-Juergen Oertel <oe@port.de>.
 *
 * Revision 1.1  2003/07/18 00:11:46  gerg
 * I followed as much rules as possible (I hope) and generated a patch for the
 * uClinux distribution. It contains an additional driver, the CAN driver, first
 * for an SJA1000 CAN controller:
 *   uClinux-dist/linux-2.4.x/drivers/char/can4linux
 * In the "user" section two entries
 *   uClinux-dist/user/can4linux     some very simple test examples
 *   uClinux-dist/user/horch         more sophisticated CAN analyzer example
 *
 * Patch submitted by Heinz-Juergen Oertel <oe@port.de>.
 *
 *
 *
 *
 *
 *--------------------------------------------------------------------------
 */


/**
* \file can_defs.h
* \author Name, port GmbH
* $Revision: 1.1 $
* $Date: 2009-01-27 04:27:11 $
*
* Module Desription
* see Doxygen Doc for all possibilites
*
*
*
*/

/* Configuration defines */
#define DEBUG 0

#define LDDK_USE_BLKREQUEST 0
#define CAN4LINUX_PCI 0
#define LDDK_USE_SYSCTL 0
#define LDDK_USE_PROCINFO 0
#define CAN_USE_FILTER 0
#define LDDK_USE_REGISTER 1
#define CAN_LOOPBACK_MODE 0
#define DELAY 1               /* 1 jeffy */


#if 0   /* auf uClinux statisch ?? */
#ifndef NOMODULE
#define __NO_VERSION__
#define MODULE
#endif
#endif

#ifdef __KERNEL__
#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

#ifdef __KERNEL__
#include <linux/init.h>
#include <linux/module.h>


#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/errno.h>
#include <linux/major.h>

#include <linux/version.h>
#include <linux/utsrelease.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,12)
# include <linux/slab.h>
#else
# include <linux/malloc.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,1,0)
#include <linux/poll.h>
#endif

#include <asm/io.h>
#include <asm/segment.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/dma.h>

#include <linux/mm.h>
#include <linux/signal.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,1,0)
#include <asm/uaccess.h>

#define __lddk_copy_from_user(a,b,c) copy_from_user(a,b,c)
#define __lddk_copy_to_user(a,b,c) copy_to_user(a,b,c)

#else

#define __lddk_copy_from_user(a,b,c) memcpy_fromfs(a,b,c)
#define __lddk_copy_to_user(a,b,c) memcpy_tofs(a,b,c)


#include <linux/sched.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,4)
#include <linux/ioport.h>
#endif
#include <linux/ioport.h>

#endif

#if CAN4LINUX_PCI
# define _BUS_TYPE "PCI-"
/* the only one supported: EMS CPC-PCI */
# define PCI_VENDOR 0x110a
# define PCI_DEVICE 0x2104

#else
# define _BUS_TYPE "ISA-"
#endif

#ifdef CAN_PELICANMODE

# ifdef  CAN_PORT_IO
#  define __CAN_TYPE__ _BUS_TYPE "PeliCAN-port I/O "
# else
#  define __CAN_TYPE__ _BUS_TYPE "PeliCAN-memory mapped "
# endif

#elif defined(CONFIG_M528x)
#  define __CAN_TYPE__ _BUS_TYPE "FlexCAN "
#else

# ifdef  CAN_PORT_IO
#  define __CAN_TYPE__ _BUS_TYPE "Philips-Basic-CAN port I/O "
# else
#  define __CAN_TYPE__ _BUS_TYPE "Philips-Basic-CAN memory mapped "
# endif

#endif

/* Length of the "version" string entry in /proc/.../version */
#define PROC_VER_LENGTH 30 
/* Length of the "Chipset" string entry in /proc/.../version */
#define PROC_CHIPSET_LENGTH 30 

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
/* kernels higher 2.3.x have a f****** new kernel interface ******************/
#define __LDDK_WRITE_TYPE	ssize_t
#define __LDDK_CLOSE_TYPE	int
#define __LDDK_READ_TYPE	ssize_t
#define __LDDK_OPEN_TYPE	int
#define __LDDK_IOCTL_TYPE	int
#define __LDDK_SELECT_TYPE	unsigned int

#define __LDDK_SEEK_PARAM 	struct file *file, loff_t off, size_t count
#define __LDDK_READ_PARAM 	struct file *file, char *buffer, size_t count, loff_t *loff
#define __LDDK_WRITE_PARAM 	struct file *file, const char *buffer, size_t count, loff_t *loff
#define __LDDK_READDIR_PARAM 	struct file *file, void *dirent, filldir_t count
#define __LDDK_SELECT_PARAM 	struct file *file, struct poll_table_struct *wait
#define __LDDK_IOCTL_PARAM 	struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg
#define __LDDK_MMAP_PARAM 	struct file *file, struct vm_area_struct * vma
#define __LDDK_OPEN_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_FLUSH_PARAM	struct file *file 
#define __LDDK_CLOSE_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_FSYNC_PARAM 	struct file *file, struct dentry *dentry, int datasync
#define __LDDK_FASYNC_PARAM 	struct file *file, int count 
#define __LDDK_CCHECK_PARAM 	kdev_t dev
#define __LDDK_REVAL_PARAM 	kdev_t dev

#define __LDDK_MINOR MINOR(file->f_dentry->d_inode->i_rdev)
#define __LDDK_INO_MINOR MINOR(inode->i_rdev)


#ifndef SLOW_DOWN_IO
# define SLOW_DOWN_IO __SLOW_DOWN_IO
#endif


#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
/* kernels higher 2.2.x have a f****** new kernel interface ******************/
#define __LDDK_READ_TYPE	ssize_t
#define __LDDK_WRITE_TYPE	ssize_t
#define __LDDK_SELECT_TYPE	unsigned int
#define __LDDK_IOCTL_TYPE	int
#define __LDDK_OPEN_TYPE	int
#define __LDDK_CLOSE_TYPE	int

#define __LDDK_SEEK_PARAM 	struct file *file, loff_t off, int count
#define __LDDK_READ_PARAM 	struct file *file, char *buffer, size_t count, loff_t *loff
#define __LDDK_WRITE_PARAM 	struct file *file, const char *buffer, size_t count, loff_t *loff
#define __LDDK_READDIR_PARAM 	struct file *file, struct dirent *dirent, int count
#define __LDDK_SELECT_PARAM 	struct file *file, poll_table *wait
#define __LDDK_IOCTL_PARAM 	struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg
#define __LDDK_MMAP_PARAM 	struct file *file, struct vm_area_struct * vma
#define __LDDK_OPEN_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_FLUSH_PARAM	struct file *file 
#define __LDDK_CLOSE_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_FSYNC_PARAM 	struct file *file
#define __LDDK_FASYNC_PARAM 	struct file *file, int count 
#define __LDDK_CCHECK_PARAM 	kdev_t dev
#define __LDDK_REVAL_PARAM 	kdev_t dev

#define __LDDK_MINOR MINOR(file->f_dentry->d_inode->i_rdev)
#define __LDDK_INO_MINOR MINOR(inode->i_rdev)

#ifndef SLOW_DOWN_IO
# define SLOW_DOWN_IO __SLOW_DOWN_IO
#endif

#else
/* kernels < 2.3.x **********************************************************/
 /* 2.0.x */
#define __LDDK_WRITE_TYPE	int
#define __LDDK_CLOSE_TYPE	void
#define __LDDK_READ_TYPE
#define __LDDK_OPEN_TYPE
#define __LDDK_IOCTL_TYPE
#define __LDDK_SELECT_TYPE	int

#define __LDDK_SEEK_PARAM 	struct inode *inode, struct file *file, off_t off, int count
#define __LDDK_READ_PARAM 	struct inode *inode, struct file *file, char *buffer, int count
#define __LDDK_WRITE_PARAM 	struct inode *inode, struct file *file, const char *buffer, int count
#define __LDDK_READDIR_PARAM 	struct inode *inode, struct file *file, struct dirent *dirent, int count
#define __LDDK_SELECT_PARAM 	struct inode *inode, struct file *filp, int sel_type, select_table * wait
#define __LDDK_IOCTL_PARAM 	struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg
#define __LDDK_MMAP_PARAM 	struct inode *inode, struct file *file, struct vm_area_struct * vma
#define __LDDK_OPEN_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_CLOSE_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_FSYNC_PARAM 	struct inode *inode, struct file *file
#define __LDDK_FASYNC_PARAM 	struct inode *inode, struct file *file, int count 
#define __LDDK_CCHECK_PARAM 	kdev_t dev
#define __LDDK_REVAL_PARAM 	kdev_t dev

#define __LDDK_MINOR MINOR(inode->i_rdev)
#define __LDDK_INO_MINOR MINOR(inode->i_rdev)

#endif

/************************************************************************/
#include "can_debug.h"
/************************************************************************/
/* #include <can_Proto.h> */
#ifdef __KERNEL__
extern int can_read (__LDDK_READ_PARAM); 
extern __LDDK_WRITE_TYPE can_write (__LDDK_WRITE_PARAM); 
extern __LDDK_SELECT_TYPE can_select ( __LDDK_SELECT_PARAM ); 
extern int can_ioctl ( __LDDK_IOCTL_PARAM ); 
extern int can_open ( __LDDK_OPEN_PARAM );
extern __LDDK_CLOSE_TYPE can_close (__LDDK_CLOSE_PARAM); 
#endif 


/*---------- Default Outc value for some known boards
 * this depends on the transceiver configuration
 * 
 * the port board uses optocoupler configuration as denoted in the philips
 * application notes, so the OUTC value is 0xfa
 *
 */

#if   defined(ATCANMINI_BASIC) || defined(ATCANMINI_PELICAN)
# define CAN_OUTC_VAL           0xfa
# define IO_MODEL		'p'
# define STD_MASK		0xFFFFFFFF 
# include "can_sja1000.h"
#elif defined(IME_SLIMLINE)
# define CAN_OUTC_VAL           0xda
# define IO_MODEL		'm'
# define STD_MASK		0xFFFFFFFF
# include "can_sja1000.h"
#elif defined(CPC_PCI)
# define CAN_OUTC_VAL           0xda
# define IO_MODEL		'm'
# define STD_MASK		0xFFFFFFFF 
# include "can_sja1000.h"
#elif defined(IXXAT_PCI03)
# define CAN_OUTC_VAL           0x5e
# define IO_MODEL		'm'
# define STD_MASK		0xFFFFFFFF
# include "can_sja1000.h"
#elif defined(PCM3680)
# define CAN_OUTC_VAL           0x5e
# define IO_MODEL		'm'
# define STD_MASK		0xFFFFFFFF 
# include "can_sja1000.h"
#elif defined(CCPC104)
# define CAN_OUTC_VAL           0xfa
# define IO_MODEL		'm'
# define STD_MASK		0xFFFFFFFF 
# include "can_sja1000.h"
#elif defined(CONFIG_M532x) || defined(CONFIG_M5253) || defined(CONFIG_M528x)
# define CAN_OUTC_VAL           0x5e
# define IO_MODEL		'm'
# define STD_MASK		0
# include "can_mcf.h"
#else 
# define CAN_OUTC_VAL           0x00
# define IO_MODEL		'm'
# define STD_MASK		0xFFFFFFFF 
# include "can_sja1000.h"
/* #error no CAN_OUTC_VAL */
#endif

/************************************************************************/
#include "can4linux.h"
/************************************************************************/
 extern volatile int irq2minormap[];
 extern volatile int irq2pidmap[];
 extern u32 Can_pitapci_control[];


/* number of supported CAN channels */
#ifndef MAX_CHANNELS
# define MAX_CHANNELS 2
#endif

/* #define MAX_BUFSIZE 64 */
#define MAX_BUFSIZE 200
/* #define MAX_BUFSIZE 4 */

#define BUF_EMPTY    0
#define BUF_OK       1
#define BUF_FULL     BUF_OK
#define BUF_OVERRUN  2
#define BUF_UNDERRUN 3


 typedef struct {
	int head;
        int tail;
        int status;
	int active;
	char free[MAX_BUFSIZE];
        canmsg_t data[MAX_BUFSIZE];
 } msg_fifo_t;



#if CAN_USE_FILTER
 #define MAX_ID_LENGTH 11
 #define MAX_ID_NUMBER (1<<11)

 typedef struct {
	unsigned    use;
	unsigned    signo[3];
	struct {
		unsigned    enable    : 1;
		unsigned    timestamp : 1;
		unsigned    signal    : 2;
		canmsg_t    *rtr_response;
	} filter[MAX_ID_NUMBER];
 } msg_filter_t;



 extern msg_filter_t Rx_Filter[];
#endif

 extern msg_fifo_t Tx_Buf[];
 extern msg_fifo_t Rx_Buf[];

 typedef irqreturn_t (*irqservice_t)(int irq, void *unused, struct pt_regs *ptregs);

 extern int Can_RequestIrq(int minor, int irq, irqservice_t handler);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,1)
 extern wait_queue_head_t CanWait[];
#else
 extern struct wait_queue *CanWait[];
#endif

 extern unsigned char *can_base[];
 extern unsigned int   can_range[];
#endif

extern int IRQ_requested[];
extern int Can_minors[];			/* used as IRQ dev_id */
extern int selfreception;			/* flag */


/************************************************************************/
/*#define LDDK_USE_SYSCTL 1 */
#ifdef __KERNEL__
#include <linux/sysctl.h>

extern ctl_table Can_sysctl_table[];
extern ctl_table Can_sys_table[];


 /* ------ Global Definitions for version */

extern char version[];
#define SYSCTL_VERSION 1
 
 /* ------ Global Definitions for Chpset */

extern char Chipset[];
#define SYSCTL_CHIPSET 2
 
 /* ------ Global Definitions for IOModel */

extern char IOModel[];
#define SYSCTL_IOMODEL 3
 
 /* ------ Global Definitions for IRQ */

extern  int IRQ[];
#define SYSCTL_IRQ 4
 
 /* ------ Global Definitions for Base */

extern  unsigned int Base[];
#define SYSCTL_BASE 5
 
 /* ------ Global Definitions for Baud */

extern  int Baud[];
#define SYSCTL_BAUD 6
 
 /* ------ Global Definitions for AccCode */

extern  unsigned int AccCode[];
#define SYSCTL_ACCCODE 7
 
 /* ------ Global Definitions for AccMask */

extern  unsigned int AccMask[];
#define SYSCTL_ACCMASK 8
 
 /* ------ Global Definitions for Timeout */

extern  int Timeout[];
#define SYSCTL_TIMEOUT 9

 /* ------ Global Definitions for Outc */

extern  int Outc[];
#define SYSCTL_OUTC 10
 
 /* ------ Global Definitions for TxErr */

extern  int TxErr[];
#define SYSCTL_TXERR 11
 
 /* ------ Global Definitions for RxErr */

extern  int RxErr[];
#define SYSCTL_RXERR 12
 
 /* ------ Global Definitions for Overrun */

extern  int Overrun[];
#define SYSCTL_OVERRUN 13
 
 /* ------ Global Definitions for dbgMask */

extern unsigned int dbgMask;
#define SYSCTL_DBGMASK 14

 /* ------ Global Definitions for Test  */
extern  int Cnt1[];
#define SYSCTL_CNT1 15
extern  int Cnt2[];
#define SYSCTL_CNT2 16

 
#endif
/************************************************************************/


#ifndef CAN_MODULE
#define CAN_MODULE 0
#endif


#ifndef Can_MAJOR
#define Can_MAJOR 91
#endif

extern int Can_errno;
extern int Can_read_flag;

#ifdef USE_LDDK_RETURN
#define LDDK_RETURN(arg) DBGout();return(arg)
#else
#define LDDK_RETURN(arg) return(arg)
#endif


/************************************************************************/
/* function prototypes */
/************************************************************************/
extern int CAN_ChipReset(int);
extern int CAN_SetTiming(int, int);
extern int CAN_StartChip(int);
extern int CAN_StopChip(int);
extern int CAN_SetMask(int, unsigned int, unsigned int);
extern int CAN_SetOMode(int,int);
#if defined(CONFIG_M528x)
extern int CAN_SetListenOnlyMode(int, int);
#endif
extern int CAN_SendMessage(int, canmsg_t *);
extern int CAN_GetMessage(int , canmsg_t *);
extern irqreturn_t CAN_Interrupt(int irq, void *unused, struct pt_regs *ptregs );
extern int CAN_VendorInit(int);

extern void register_systables(void);
extern void unregister_systables(void);

/* can_82c200funcs.c */
extern int CAN_ShowStat (int board);

/* can_mcffuncs.c */
void mcf_irqreset(void);
void mcf_irqsetup(void);

/* util.c */
extern int Can_FifoInit(int minor);
extern int Can_FilterCleanup(int minor);
extern int Can_FilterInit(int minor);
extern int Can_FilterMessage(int minor, unsigned message, unsigned enable);
extern int Can_FilterOnOff(int minor, unsigned on);
extern int Can_FilterSigNo(int minor, unsigned signo, unsigned signal);
extern int Can_FilterSignal(int minor, unsigned id, unsigned signal);
extern int Can_FilterTimestamp(int minor, unsigned message, unsigned stamp);
extern int Can_FreeIrq(int minor, int irq );
extern int Can_WaitInit(int minor);
extern void Can_StartTimer(unsigned long v);
extern void Can_StopTimer(void);
extern void Can_TimerInterrupt(unsigned long unused);
extern void Can_dump(int minor);
extern void Can_dump(int minor);
extern void CAN_register_dump(int);
extern void CAN_object_dump(int object);
extern void print_tty(const char *fmt, ...);

/* PCI support */
extern int pcimod_scan(void);

/************************************************************************/
/* hardware access functions or macros */
/************************************************************************/
#ifdef  CAN_PORT_IO
/* #error Intel port I/O access */
/* using port I/O with inb()/outb() for Intel architectures like 
   AT-CAN-MINI ISA board */

#ifdef IODEBUG
#  define CANout(bd,adr,v)	\
	(printk("Cout: (%x)=%x\n", (int)&((canregs_t *)Base[bd])->adr, v), \
		outb(v, (int) &((canregs_t *)Base[bd])->adr ))
#else
#  define CANout(bd,adr,v)	\
		(outb(v, (int) &((canregs_t *)Base[bd])->adr ))
#endif
#define CANin(bd,adr)		\
		(inb ((int) &((canregs_t *)Base[bd])->adr  ))
#define CANset(bd,adr,m)	\
	outb((inb((int) &((canregs_t *)Base[bd])->adr)) \
		| m ,(int) &((canregs_t *)Base[bd])->adr )
#define CANreset(bd,adr,m)	\
	outb((inb((int) &((canregs_t *)Base[bd])->adr)) \
		& ~m,(int) &((canregs_t *)Base[bd])->adr )
#define CANtest(bd,adr,m)	\
	(inb((int) &((canregs_t *)Base[bd])->adr  ) & m )

#else 	/* CAN_PORT_IO */
/* using memory acces with readb(), writeb() */
/* #error  memory I/O access */
/* #define can_base Base */
#ifdef IODEBUG
#  define CANout(bd,adr,v)	\
	(printk("Cout: (%x)=%x\n", (u32)&((canregs_t *)can_base[bd])->adr, v), \
		writeb(v, (u32) &((canregs_t *)can_base[bd])->adr ))

#define CANset(bd,adr,m)     do	{\
	unsigned char v;	\
        v = (readb((u32) &((canregs_t *)can_base[bd])->adr)); \
	printk("CANset %x |= %x\n", (v), (m)); \
	writeb( v | (m) , (u32) &((canregs_t *)can_base[bd])->adr ); \
	} while (0)

#define CANreset(bd,adr,m)	do {\
	unsigned char v; \
        v = (readb((u32) &((canregs_t *)can_base[bd])->adr)); \
	printk("CANreset %x &= ~%x\n", (v), (m)); \
	writeb( v & ~(m) , (u32) &((canregs_t *)can_base[bd])->adr ); \
	} while (0)

#else
   /* Memory Byte access */
#define CANout(bd,adr,v)	\
		(writeb(v, (u32) &((canregs_t *)can_base[bd])->adr ))
#define CANset(bd,adr,m)	\
	writeb((readb((u32) &((canregs_t *)can_base[bd])->adr)) \
		| (m) , (u32) &((canregs_t *)can_base[bd])->adr )
#define CANreset(bd,adr,m)	\
	writeb((readb((u32) &((canregs_t *)can_base[bd])->adr)) \
		& ~(m), (u32) &((canregs_t *)can_base[bd])->adr )
#endif
#define CANin(bd,adr)		\
		(readb ((u32) &((canregs_t *)can_base[bd])->adr  ))
#define CANtest(bd,adr,m)	\
	(readb((u32) &((canregs_t *)can_base[bd])->adr  ) & (m) )


   /* Memory word access */
#define CANoutw(bd,adr,v)	\
		(writew(v, (u32) &((canregs_t *)can_base[bd])->adr ))
#define CANsetw(bd,adr,m)	\
	writew((readw((u32) &((canregs_t *)can_base[bd])->adr)) \
		| (m) , (u32) &((canregs_t *)can_base[bd])->adr )
#define CANresetw(bd,adr,m)	\
	writew((readw((u32) &((canregs_t *)can_base[bd])->adr)) \
		& ~(m), (u32) &((canregs_t *)can_base[bd])->adr )
#define CANinw(bd,adr)		\
		(readw ((u32) &((canregs_t *)can_base[bd])->adr  ))
#define CANtestw(bd,adr,m)	\
	(readw((u32) &((canregs_t *)can_base[bd])->adr  ) & (m) )


   /* Memory long word access */
#define CANoutl(bd,adr,v)	\
		(writel(v, (u32) &((canregs_t *)can_base[bd])->adr ))
#define CANsetl(bd,adr,m)	\
	writel((readl((u32) &((canregs_t *)can_base[bd])->adr)) \
		| (m) , (u32) &((canregs_t *)can_base[bd])->adr )
#define CANresetl(bd,adr,m)	\
	writel((readl((u32) &((canregs_t *)can_base[bd])->adr)) \
		& ~(m), (u32) &((canregs_t *)can_base[bd])->adr )
#define CANinl(bd,adr)		\
		(readl ((u32) &((canregs_t *)can_base[bd])->adr  ))
#define CANtestl(bd,adr,m)	\
	(readl((u32) &((canregs_t *)can_base[bd])->adr  ) & (m) )


#endif 		/* CAN_PORT_IO */

/*________________________E_O_F_____________________________________________*/









