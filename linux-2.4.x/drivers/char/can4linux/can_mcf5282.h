/*
 * can_mc5282.h - can4linux CAN driver module
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (c) 2003 port GmbH Halle/Saale
 *------------------------------------------------------------------
 * $Header: Exp $
 *
 *--------------------------------------------------------------------------
 *
 *
 * modification history
 * --------------------
 * $Log: $
 *
 *
 *
 *--------------------------------------------------------------------------
 */
 /*


MCF5282DE
Rev. 1.5, 08/2004


8         32-bit Accesses to FlexCAN Registers do not
          Work Properly
8.1       Description
Since the FlexCAN was originally designed for 16-bit architectures,
all 32-bit register accesses are broken down into two back-to-back 16-bit accesses.
However, the timing for the back-to-back accesses is incorrect
and leads to corruption of the second 16-bit read or write.
8.2       Workaround
When reading or writing to the 32-bit RxMASK registers,
use two 16-bit accesses instead of a single 32-bit access.
DATECODES AFFECTED: All






 */


extern unsigned int Base[];

#ifdef CONFIG_CLOCK_80MHz
#define CAN_SYSCLK 40
#else
#define CAN_SYSCLK 32
#endif

/* define some types, header file comes from CANopen */
#define UNSIGNED8 u8
#define UNSIGNED16 u16


#define MCFFLEXCAN_BASE (MCF_MBAR + 0x1c0000)	/* Base address FlexCAN module */


/* can4linux does not use all the full CAN features, partly because it doesn't
   make sense.
 */

/* We use only one transmit object for all messages to be transmitted */
#define TRANSMIT_OBJ 0
#define RECEIVE_STD_OBJ 1
#define RECEIVE_EXT_OBJ 2
#define RECEIVE_RTR_OBJ 3


#include "TouCAN.h"



