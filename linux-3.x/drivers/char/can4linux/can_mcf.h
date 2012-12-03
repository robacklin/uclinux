/*
 * can_mcf.h - can4linux CAN driver module
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (c) 2003 port GmbH Halle/Saale
 *------------------------------------------------------------------
 *
 *
 * modification history
 * --------------------
 * Revision 1.1  2005/03/15 12:29:16  vvorobyov
 * CAN support added 2.6 kernel.
 *
 *
 *
 *
 *--------------------------------------------------------------------------
 */


extern unsigned int Base[];


#ifdef CONFIG_M532x
 #define CAN_SYSCLK 16
 /* If internal clock is used, than frequency should be 80Mhz (Fsys/3) */
 /* #define CAN_SYSCLK 80 */
#elif defined(CONFIG_M5253)
 #define CAN_SYSCLK 70
#else
 #define CAN_SYSCLK 32
#endif

#if defined(CONFIG_UC532X) || defined(CONFIG_UC53281EVM)
#ifdef CAN_SYSCLK
#undef CAN_SYSCLK
#endif
#define CAN_SYSCLK 80
#endif

#ifdef CONFIG_M528x
#ifdef CAN_SYSCLK
#undef CAN_SYSCLK
#endif
#if defined(CONFIG_CLOCK_80MHz) || (CONFIG_CLOCK_FREQ == 80000000)
#define CAN_SYSCLK 40
#else
#define CAN_SYSCLK 32
#endif
#define MCFFLEXCAN_BASE (MCF_MBAR + 0x1c0000)   /* Base address FlexCAN module */
#endif

/* define some types, header file comes from CANopen */
#define UNSIGNED8 u8
#define UNSIGNED16 u16


/* can4linux does not use all the full CAN features, partly because it doesn't
   make sense.
 */

/* We use only one transmit object of all messages to be transmitted */
#define TRANSMIT_OBJ		0
#define RECEIVE_STD_OBJ		1
#define RECEIVE_EXT_OBJ		2
#define RECEIVE_RTR_OBJ		3


#ifdef CONFIG_FLEXCAN_CAN0
#define		CAN_MODULE	0 
#endif 

#ifdef CONFIG_FLEXCAN_CAN1
#define		CAN_MODULE	1 
#endif 

#include "TouCAN.h"
#include <asm/coldfire.h>

#ifdef CONFIG_M532x
#include <asm/m532xsim.h>
#endif

#ifdef CONFIG_M5253
#include <asm/m5253sim.h>
#endif
