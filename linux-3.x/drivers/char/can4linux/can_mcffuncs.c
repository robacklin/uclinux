/* can_mcffuncs.c  - Motorola FlexCAN functions
 *
 * can4linux -- LINUX CAN device driver source
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 *
 * Copyright (c) 2003 port GmbH Halle/Saale
 * (c) 2003 Heinz-Jrgen Oertel (oe@port.de)
 *------------------------------------------------------------------
 *
 *  TRAMSMIT_OBJ  - used to transmit messages, possible are:
 *  	standard, extended, standard RTR, extended RTR
 *  RECEIVE_RTR_OBJ
 *  RECEIVE_STD_OBJ
 *  RECEIVE_EXT_OBJ
 *
 *
 * modification history
 * --------------------
 * Revision 1.1  2005/03/15 12:29:16  vvorobyov
 * CAN support added 2.6 kernel.
 *
 *
 *
 */
#include "can_defs.h"
#include <asm/delay.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
# error use the Coldfire FLexCAN only with Kernel > 2.4
#endif

/* int	CAN_Open = 0; */

/* timing values */
static const BTR_TAB_TOUCAN_T can_btr_tab_toucan[] = {
#if CAN_SYSCLK != 70 && CAN_SYSCLK != 80
 { 10, CAN_PRESDIV_10K, CAN_PROPSEG_10K, CAN_PSEG1_10K, CAN_PSEG2_10K },
#endif
 { 20, CAN_PRESDIV_20K, CAN_PROPSEG_20K, CAN_PSEG1_20K, CAN_PSEG2_20K },
 { 50, CAN_PRESDIV_50K, CAN_PROPSEG_50K, CAN_PSEG1_50K, CAN_PSEG2_50K },
 { 100, CAN_PRESDIV_100K, CAN_PROPSEG_100K, CAN_PSEG1_100K, CAN_PSEG2_100K },
 { 125, CAN_PRESDIV_125K, CAN_PROPSEG_125K, CAN_PSEG1_125K, CAN_PSEG2_125K },
 { 250, CAN_PRESDIV_250K, CAN_PROPSEG_250K, CAN_PSEG1_250K, CAN_PSEG2_250K },
 { 500, CAN_PRESDIV_500K, CAN_PROPSEG_500K, CAN_PSEG1_500K, CAN_PSEG2_500K },
#if CAN_SYSCLK != 70	/* For MCF5253EVB */
 { 800, CAN_PRESDIV_800K, CAN_PROPSEG_800K, CAN_PSEG1_800K, CAN_PSEG2_800K },
#endif
 { 1000, CAN_PRESDIV_1000K, CAN_PROPSEG_1000K, CAN_PSEG1_1000K, CAN_PSEG2_1000K },
 {0, 0, 0, 0, 0}  /* last entry */
};




/* Board reset
   means the following procedure:
  set Reset Request
  wait for Rest request is changing - used to see if board is available
  initialize board (with valuse from /proc/sys/Can)
    set output control register
    set timings
    set acceptance mask
*/

int CAN_ShowStat (int board)
{
#if DEBUG
    if (dbgMask && (dbgMask & DBG_DATA)) {
    printk(" CANMCR 0x%lx,", MCF_FLEXCAN_CANMCR(board));
    printk(" ERRSTAT 0x%lx,", MCF_FLEXCAN_ERRSTAT(board));
    printk(" IFLAGS 0x%lx,", MCF_FLEXCAN_IFLAG(board));
    printk(" IMASK 0x%lx,", MCF_FLEXCAN_IMASK(board));
    printk("\n");
    }
#endif
    return 0;
}


/* can_GetStat - read back as many status information as possible
*
* Because the CAN protocol itself describes different kind of information
* already, and the driver has some generic information
* (e.g about it's buffers)
* we can define a more or less hardware independent format.
*
*
* exception:
* ERROR WARNING LIMIT REGISTER (EWLR)
* The SJA1000 defines a EWLR, reaching this Error Warning Level
* an Error Warning interrupt can be generated.
* The default value (after hardware reset) is 96. In reset
* mode this register appears to the CPU as a read/write
* memory. In operating mode it is read only.
* Note, that a content change of the EWLR is only possible,
* if the reset mode was entered previously. An error status
* change (see status register; Tablecan_core.c 14) and an error
* warning interrupt forced by the new register content will not
* occur until the reset mode is cancelled again.
*/

int can_GetStat(
	struct inode *inode,
	CanStatusPar_t *stat
	)
{
unsigned int board = MINOR(inode->i_rdev);
msg_fifo_t *Fifo;
unsigned long flags;


    stat->type = CAN_TYPE_FlexCAN;

    stat->baud = Baud[board];
    stat->status = MCF_FLEXCAN_ERRSTAT(board);
    /* not available in the FlexCAN, lets fill 127 here */
    stat->error_warning_limit = 127;
    stat->rx_errors = (unsigned int)((MCF_FLEXCAN_ERRCNT(board)&0xFF00)>>8);
    stat->tx_errors = (unsigned int)(MCF_FLEXCAN_ERRCNT(board)&0x00FF);
    /* error code is not available, use estat again */
    stat->error_code= MCF_FLEXCAN_ERRSTAT(board);

    /* Disable CAN Interrupts */
    /* save_flags(flags); cli(); */
	local_irq_save(flags);
    Fifo = &Rx_Buf[board];
    stat->rx_buffer_size = MAX_BUFSIZE;	/**< size of rx buffer  */
    /* number of messages */
    stat->rx_buffer_used =
    	(Fifo->head < Fifo->tail)
    	? (MAX_BUFSIZE - Fifo->tail + Fifo->head) : (Fifo->head - Fifo->tail);
    Fifo = &Tx_Buf[board];
    stat->tx_buffer_size = MAX_BUFSIZE;	/* size of tx buffer  */
    /* number of messages */
    stat->tx_buffer_used =
    	(Fifo->head < Fifo->tail)
    	? (MAX_BUFSIZE - Fifo->tail + Fifo->head) : (Fifo->head - Fifo->tail);
    /* Enable CAN Interrupts */
    /* restore_flags(flags); */
	local_irq_restore(flags);
    return 0;
}



/*
 * CAN_ChipReset - performs the first initialization or re-iniatalization of the chip
 *
 *
 */
int CAN_ChipReset (int board)
{
int i;

    DBGin("CAN_ChipReset " __TIME__);

    /*
     * Initialize Port AS PAR to have Can TX/RX signals enabled
     */
    /*
     * go to INIT mode
     * Any configuration change/initialization requires that the FlexCAN be
     * frozen by either asserting the HALT bit in the
     * module configuration register or by reset.
     * For Init_CAN() we choose reset.
     */

#if (defined(CONFIG_M532x) && CAN_SYSCLK == 80) || defined(CONFIG_M5253)
    /* Select the clock source for can module */
    MCF_FLEXCAN_CANMCR(board) = ( MCF_FLEXCAN_CANMCR(board)
    					| CANMCR_MDIS
					);

    udelay(10);

    MCF_FLEXCAN_CANCTRL(board) = ( MCF_FLEXCAN_CANCTRL(board)
    					| CANCTRL_CLKSRC
					);
#if DEBUG
    printk("CANMCR=%lx CANCTRL=%lx\n",
    		MCF_FLEXCAN_CANMCR(board),
			MCF_FLEXCAN_CANCTRL(board));
#endif
#endif

    MCF_FLEXCAN_CANMCR(board) = ( MCF_FLEXCAN_CANMCR(board)
                          &(~CANMCR_MDIS)
                          );
    MCF_FLEXCAN_CANMCR(board) = ( MCF_FLEXCAN_CANMCR(board)
                          &(~CANMCR_FRZ)
                         );
    MCF_FLEXCAN_CANMCR(board) = ( MCF_FLEXCAN_CANMCR(board)
                          |CANMCR_SOFTRST
                         );
    udelay(10);
    /* Test Reset Status */
    if((MCF_FLEXCAN_CANMCR(board) & CANMCR_SOFTRST)!= 0) {
	DBGout();return -1;
    }

    udelay(10);
    /* Initialize the transmit and receive pin modes in control register 0 */
    MCF_FLEXCAN_CANCTRL(board) = ( MCF_FLEXCAN_CANCTRL(board)
                             &(~(CANCTRL_BOFFMSK | CANCTRL_ERRMSK))
                            );


    CAN_SetTiming(board, Baud[board]);

    /*
     * Select the internal arbitration mode
     * LBUF in CANCTRL
     * LBUF Lowest Buffer Transmitted First
     * The LBUF bit defines the transmit-first scheme.
     * 0 = Message buffer with lowest ID is transmitted first.
     * 1 = Lowest numbered buffer ist transmitted first.
     *
     * should have no impact here, the driver is using only one
     * TX object
     *
     * !! change the order of the next two statements  !!
     */

    MCF_FLEXCAN_CANCTRL(board) = ( MCF_FLEXCAN_CANCTRL(board)
                           |CANCTRL_LBUF
                          );
    MCF_FLEXCAN_CANCTRL(board) = ( MCF_FLEXCAN_CANCTRL(board)
                           &(~CANCTRL_LBUF)
                          );

    /*
     * Initialize message buffers.
     * The control/status word of all message buffers are written
     * as an inactive receive message buffer.
     */
    for(i = 0; i < 16; i++) {
	  MCF_FLEXCAN_MB_CNT(board,i) = MB_CNT_CODE(REC_CODE_NOT_ACTIVE)
	                                   | MB_CNT_LENGTH(0) ;
    }

    /* some receive objects
     * - RECEIVE_STD_OBJ to receive standard data frames
     * - ID = 0
     */
    MCF_FLEXCAN_MB_ID(board,RECEIVE_STD_OBJ) = MB_ID_STD(0);
    MCF_FLEXCAN_MB_CNT(board,RECEIVE_STD_OBJ) = ( MB_CNT_CODE(REC_CODE_EMPTY)
                                       | MB_CNT_LENGTH(8)
                                       );
    /* some receive objects
     * - RECEIVE_EXT_OBJ to receive extended data frames
     * - ID = 0
     */
    MCF_FLEXCAN_MB_ID(board,RECEIVE_EXT_OBJ) = MB_ID_EXT(0);
    MCF_FLEXCAN_MB_CNT(board,RECEIVE_EXT_OBJ) = ( MB_CNT_CODE(REC_CODE_EMPTY)
                                       | MB_CNT_LENGTH(8)
				       | MB_CNT_IDE
                                       );
    /* create a receive object
     * - RECEIVE_RTR_OBJ to receive standard remote frame and to request on it
     * - ID = 100
     */
    MCF_FLEXCAN_MB_CNT(board,RECEIVE_RTR_OBJ) = ( MB_CNT_CODE(TRANS_CODE_NOT_READY)
                                       | MB_CNT_LENGTH(0)
				       );
    MCF_FLEXCAN_MB_ID(board,RECEIVE_RTR_OBJ) = MB_ID_STD(100);
    /* data that will be transmitted uncondicionally */
    for(i = 0; i < 8; i++)
		MCF_FLEXCAN_MB_DB(board,RECEIVE_RTR_OBJ,i) = 0x55;

    MCF_FLEXCAN_MB_CNT(board,RECEIVE_RTR_OBJ) = (
		MB_CNT_CODE(TRANS_CODE_TRANSMIT_ONLY_RTR)
		| MB_CNT_LENGTH(5));

    CAN_SetMask  (board, AccCode[board], AccMask[board] );
    /*
     * - Initialize IARB[3:0] to a non zero value in CANMCR
     *   ( This is done in CAN-ChipReset() )
     * - Set the required mask bits in the IMASK register (for all message
     *  buffer interrupts) in CANCTRL0 for bus off and error interrupts,
     *  and in CANMCR for WAKE interrupt.
     */

    /* dont't forget error int's ! */

    MCF_FLEXCAN_IMASK(board) = ( IMASK_BUFnM(TRANSMIT_OBJ)
                         | IMASK_BUFnM(RECEIVE_STD_OBJ)
			 | IMASK_BUFnM(RECEIVE_EXT_OBJ)
			 | IMASK_BUFnM(RECEIVE_RTR_OBJ)
			 );

    /* Enable the loop-back mode */
#if CAN_LOOPBACK_MODE
    MCF_FLEXCAN_CANCTRL(board) = ( MCF_FLEXCAN_CANCTRL(board)
    					| CANCTRL_LPB
					);
#endif

    CAN_register_dump(board);
    DBGout();
    return 0;
}

/* change the bit timings of the selected CAN channel */
int CAN_SetTiming (int board, int baud)
{
   /* int custom=0; */
   BTR_TAB_TOUCAN_T * table = (BTR_TAB_TOUCAN_T*)can_btr_tab_toucan;

    DBGin("CAN_SetTiming");
    DBGprint(DBG_DATA, ("baud[%d]=%d", board, baud));
    /* enable changing of bit timings */
    MCF_FLEXCAN_CANMCR(board) = ( MCF_FLEXCAN_CANMCR(board)
                          |CANMCR_HALT
                          );
    /* search for data from table */
    while(1) {
        if (table->rate == 0 || table->rate == baud) {
    	    break;
    	}
    	table++;
    }
    if (table->rate == 0) {
	/* try to use baud  as custom specific bit rate
	 * not implemented yet
	 */
	return -ENXIO;
    }

    /*
     * Set Timing Register values.
     * Initialize the bit timing parameters PROPSEG, PSEG1, PSEG2 and RJW
     * in control registers 1 and 2.
     *
     * The TouCAN module uses three 8-bit registers to set-up
     * the bit timing parameters required by the CAN protocol.
     * Control registers 1 and 2 contain the PROPSEG, PSEG1, PSEG2
     * and RJW fields which allow the user to configure
     * the bit timing parameters.
     * The prescaler divide register (PRESDIV) allows the user to select
     * the ratio used to derive the S-Clock from the system clock.
     * The time quanta clock operates at the S-clock frequency.
     */
    MCF_FLEXCAN_CANCTRL(board) &= ~(CANCTRL_PRESDIV(0xFF) 
			   | CANCTRL_PSEG1(0x07) 
                           | CANCTRL_PSEG2(0x07)
                           | CANCTRL_PROPSEG(0x07)
                           );

    MCF_FLEXCAN_CANCTRL(board) = ( (MCF_FLEXCAN_CANCTRL(board)
                                        & (~(CANCTRL_SAMP
					  |CANCTRL_TSYNC
					  |CANCTRL_LBUF)))
                           | CANCTRL_PRESDIV(table->presdiv)
                           | CANCTRL_PSEG1(table->pseg1)
                           | CANCTRL_PSEG2(table->pseg2)
                           | CANCTRL_PROPSEG(table->propseg)
                           );


    /*
     * Stay in configuration mode; a call to Start-CAN() is necessary to
     * activate the CAN controller with the new bit rate
     */
    DBGprint(DBG_DATA,("canctrl=0x%lx presdiv=0x%lx",
    		MCF_FLEXCAN_CANCTRL(board), ((MCF_FLEXCAN_CANCTRL(board) & CANCTRL_PRESDIV(0xFF))>>24)));

    DBGout();
    return 0;
}


int CAN_StartChip (int board)
{
    RxErr[board] = TxErr[board] = 0L;
    DBGin("CAN_StartChip");

    /* clear interrupts */
    MCF_FLEXCAN_IFLAG(board) = IFLAG_BUFF_SET_ALL;
    MCF_FLEXCAN_CANMCR(board) = ( MCF_FLEXCAN_CANMCR(board)
                          &(~(CANMCR_HALT | CANMCR_FRZ))
                          );

    
    /* Interrupts on Rx, TX, any Status change and data overrun */
    /* Initialize the transmit and receive pin modes in control register 0 */
    MCF_FLEXCAN_CANCTRL(board) = ( MCF_FLEXCAN_CANCTRL(board)
                           | CANCTRL_BOFFMSK
                           | CANCTRL_ERRMSK
                           );

    /* to do */
    CAN_register_dump(board);

    DBGout();
    return 0;
}


/* Disable all CAN activities */
int CAN_StopChip (int board)
{
    msg_fifo_t *TxFifo;
    int i = MAX_BUFSIZE;
    TxFifo = &Tx_Buf[board];

    DBGin("CAN_StopChip");

    /* delay for transmission, up to BUF_EMPTY/10 ms */
    while(TxFifo->free[TxFifo->tail] != BUF_EMPTY && i) {
        i /= 10;
        udelay(1000);
    }

    MCF_FLEXCAN_CANMCR(board) = ( MCF_FLEXCAN_CANMCR(board)
                          | CANMCR_HALT | CANMCR_FRZ
                          );

    while(!(MCF_FLEXCAN_CANMCR(board) & CANMCR_FRZACK))
    	;

    MCF_FLEXCAN_CANCTRL(board) = ( MCF_FLEXCAN_CANCTRL(board)
                           &(~(CANCTRL_BOFFMSK | CANCTRL_ERRMSK))
                           );
    DBGout();
    return 0;
}

/* set value of the output control register */
int CAN_SetOMode (int board, int arg)
{

    DBGin("CAN_SetOMode");
	  DBGprint(DBG_DATA,("[%d] outc=0x%x", board, arg));

    DBGout();
    return 0;
}


/* FlexCAN only knows a 'mask' value, code is ignored */
int CAN_SetMask (int board, unsigned int code, unsigned int mask)
{

    DBGin("CAN_SetMask");
    DBGprint(DBG_DATA,("[%d] acc=0x%x mask=0x%x",  board, code, mask));
    MCF_FLEXCAN_RXGMASK(board) = mask;

    DBGout();
    return 0;
}

static int last_tx_id;

int CAN_SendMessage (int board, canmsg_t *tx)
{
    int i;
    volatile unsigned long stat;
    msg_fifo_t *TxFifo;

    TxFifo = &Tx_Buf[board];
   
    DBGin("CAN_SendMessage");

     /*
    IDLE - Idle Status. The IDLE bit indicates, when there is activity
    on the CAN bus
    1 - The CAn bus is Idle

    TX/RX - Transmission/receive status. Indicates when the FlexCAN module
    is transmitting or receiving a message. TX/RX has no meaning,
    when IDLE = 1
    0 - FlexCAN is receiving when IDLE = 0
    1 - FlexCAN is transmitting when IDLE = 0
    */

    while (
    	    ((stat = MCF_FLEXCAN_ERRSTAT(board)) &
            (ERRSTAT_IDLE | ERRSTAT_TXRX)) == ERRSTAT_TXRX)
	{
		if( need_resched() ) schedule();
    	}

    /* DBGprint(DBG_DATA,( */
    		/* "CAN[%d]: tx.flags=%d tx.id=0x%lx tx.length=%d stat=0x%x", */
		/* board, tx->flags, tx->id, tx->length, stat)); */

    TxFifo->active = 1;

    /* Writing Control/Status word to hold TX Message Object inactive */
    MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = (
		MB_CNT_CODE(TRANS_CODE_NOT_READY)
		| MB_CNT_LENGTH(0));

    /* fill the frame info and identifier fields , ID-Low and ID-High */
	if(tx->flags & MSG_EXT) {
    	/* use ID in extended message format */
		DBGprint(DBG_DATA, ("---> send ext message \n"));
		if( tx->flags & MSG_RTR) {
		      MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = (
			MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ)
			| MB_CNT_IDE
			| MB_CNT_RTR
			| MB_CNT_SRR);
		MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ) = MB_ID_EXT(tx->id);
	} else {
		MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = (
			MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ)
			|MB_CNT_IDE);
		MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ) = MB_ID_EXT(tx->id);
	}
	last_tx_id = MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ);
    } else {
	DBGprint(DBG_DATA, ("---> send std message \n"));
	
	if( tx->flags & MSG_RTR) {
		MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = (
			MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ)
			|MB_CNT_RTR);
		MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ) = MB_ID_STD(tx->id);
	} else {
		MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = (
		MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ));
		MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ) = MB_ID_STD(tx->id);
	}
	last_tx_id = MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ) >> 18;
    }

    /* - fill data ---------------------------------------------------- */
    for(i = 0; i < tx->length; i++) {
    	MCF_FLEXCAN_MB_DB(board,TRANSMIT_OBJ,i) = tx->data[i];
    }
    
    /* Writing Control/Status word (active code, length) */
    MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) &= ~ (MB_CNT_LENGTH(0x0f));
    MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = ( MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ)
                                        | MB_CNT_CODE(TRANS_CODE_TRANSMIT_ONCE)
                                        | MB_CNT_LENGTH(tx->length)
                                        );

    TxFifo->free[TxFifo->tail] = BUF_EMPTY; /* now this entry is EMPTY */
    TxFifo->tail = ++(TxFifo->tail) % MAX_BUFSIZE;
    /* - /end --------------------------------------------------------- */

    CAN_ShowStat(board);

    DBGout();return i;
}


/* look if one of the receive message objects has something received */
int CAN_GetMessage (int board, canmsg_t *rx )
{
volatile unsigned long stat;
int i = 0;
volatile unsigned long ctrl;

    DBGin("CAN_GetMessage");
    stat = MCF_FLEXCAN_ERRSTAT(board);
    DBGprint(DBG_DATA,("0x%x: stat=0x%lx iflag=0x%lx imask=0x%lx" ,
    			Base[board], stat,
    			MCF_FLEXCAN_IFLAG(board),
    			MCF_FLEXCAN_IMASK(board)));

    rx->flags  = 0;
    rx->length = 0;

    i = MCF_FLEXCAN_IFLAG(board);
    if( i & (1 << RECEIVE_STD_OBJ)) {
	/* reading the control status word of the identified message
	 * buffer triggers a lock for that buffer.
	 * A new received message frame which maches the message buffer
	 * can not be written into this buffer while it is locked
	 */
        while (((ctrl = MCF_FLEXCAN_MB_CNT(board,RECEIVE_STD_OBJ)) & (REC_CODE_BUSY << 24)) == (REC_CODE_BUSY << 24)) {
	    /* 20 cycles maximum wait */
	    /* printf1("CAN_int, rx REC_CODE_BUSY"); */
	}
	/* printk("message received 0x%x\n", ctrl); */
	rx->length = (unsigned short)((ctrl & MB_CNT_LENGTH(0xF))>>16);
	rx->id =  (MCF_FLEXCAN_MB_ID(board,RECEIVE_STD_OBJ) >> 18);
	memcpy((void*)&rx->data[0], (void*)&(MCF_FLEXCAN_MB_DB(board,RECEIVE_STD_OBJ, 0)), 8);

	/* clear interrupts */
	/* Int is cleared when the CPU reads the intrerupt flag register iflag
	 * while the associated bit is set, and then writes it back as '1'
	 * (and no new event of the same type occurs
	 * between the read and write action.)
	 * ! This is opposit to the TouCAN module, where the iflag bit
	 * has to be written back with '0'
	 */
  MCF_FLEXCAN_IFLAG(board) = IFLAG_BUFnM(RECEIVE_STD_OBJ);
	/* Reset message object */
  MCF_FLEXCAN_MB_CNT(board,RECEIVE_STD_OBJ) = ( MB_CNT_CODE(REC_CODE_EMPTY)
                                               | MB_CNT_LENGTH(8)
                                               );
	/* reading the free running timer will unlock any message buffers */
   MCF_FLEXCAN_TIMER(board) = MCF_FLEXCAN_TIMER(board);
	i = 1; /* signal one received message */
    } else {
	i = 0;
    }

    DBGout();
    return i;

}

int CAN_VendorInit (int minor)
{
    DBGin("CAN_VendorInit");
/* 1. Vendor specific part ------------------------------------------------ */
    can_range[minor] = 0x180;

/* End: 1. Vendor specific part ------------------------------------------- */

    /* Request the controllers address space */

    /* looks like not needed in uClinux with internal ressources ? */
    /* It's Memory I/O */
    if(NULL == request_mem_region(Base[minor], can_range[minor], "CAN-IO")) {
   	return -EBUSY;
    }

    /* not necessary in uClinux, but ... */
    can_base[minor] = ioremap(Base[minor], can_range[minor]);
    /* now the virtual address can be used for the register address macros */

/* 2. Vendor specific part ------------------------------------------------ */



/* End: 2. Vendor specific part ------------------------------------------- */

    if( IRQ[minor] > 0 ) {
            if( Can_RequestIrq( minor, IRQ[minor] , CAN_Interrupt) ) {
	     printk("Can[%d]: Can't request IRQ %d \n", minor, IRQ[minor]);
	     DBGout(); return -EBUSY;
        }
    }
    DBGout(); return 1;

}


/*
 * The plain CAN interrupt
 *
 *				RX ISR           TX ISR
 *                              8/0 byte
 *                               _____            _   ___
 *                         _____|     |____   ___| |_|   |__
 *---------------------------------------------------------------------------
 * 1) Motorola ColdFire 548x
 *  100 MHz, xxxxxx bogomips
 *                              __/__s            __ s
 *
 * 1) 1Byte = (42-27)/8 =      s
 * 2) 1Byte = (24-12)/8 =      s
 *
 *
 *
 * RX Int with to Receive channels:
 * 1)                _____   ___
 *             _____|     |_|   |__
 *                   30    5  20  s
 *   first ISR normal length,
 *   time between the two ISR -- short
 *   sec. ISR shorter than first, why? it's the same message
 */

irqreturn_t CAN_Interrupt ( int irq, void *dev_id, struct pt_regs *ptregs )
{
	int board;
	volatile unsigned long   estat;
	volatile unsigned long	ctrl;
	volatile int irqsrc, dummy;
	msg_fifo_t *RxFifo;
	msg_fifo_t *TxFifo;
	int i;
	unsigned long flags;
	int mask;

    board = *(int *)dev_id;
    mask = MCF_FLEXCAN_IMASK(board);

    RxFifo = &Rx_Buf[board];
    TxFifo = &Tx_Buf[board];

    /* One of the following can actually occur:
       BusOff
       ErrInt - in this case:
       	bits 4-5 - FCS - Fault confinment state tells us
       	 00 - all ok, error active mode
       	 01 - error passive mode
        bit 8 - RX warning level reached
        bit 9 - Tx warning level reached

       At the moment we aren't interested in more information

       all bits in estat are read only except for
       BOFF_INT and ERR_INT which are interrupt sources
       and can be written by the host to '1' to reset Interrupt
       conditions.
    */
    /* error interrupts */
    estat = MCF_FLEXCAN_ERRSTAT(board);
    if (estat &
       (ERRSTAT_BOFFINT | ERRSTAT_ERRINT))  {
	/* we have an error interrupt
	 * later on we can move this error handling at the end of the ISR
	 * to heve better RX response times */
	/* printk(" error status 0x%04x \n", estat); */
	/* reset all error conditions, we have saved them in estat
	 * BusOff disables the Interrupt generation itself by resetting
	 * the mask in canctrl0.
	 * ErrInt - to clear this bit, first read it (already done)
	 * then write as a zero.
	 *
	 * we do reset all possible interrupts and look what we got later
	 */
        MCF_FLEXCAN_ERRSTAT(board) = ERRSTAT_BOFFINT | ERRSTAT_ERRINT;

        /*MCF_FLEXCAN_ERRSTAT(board) = ( estat
                         &(~(ERRSTAT_BOFFINT  ))); */
        /* check for Bus Off condition */
	if (estat & ERRSTAT_BOFFINT) {
	    CAN_StopChip (board);
	    printk("We are BusOff now\n");
	}
    }
    /* check for message object interrupts */
    irqsrc = MCF_FLEXCAN_IFLAG(board);
    if(irqsrc == 0) {
         /* first call to ISR, it's not for me ! */
	return IRQ_HANDLED; 
    }
    MCF_FLEXCAN_IMASK(board) = 0; /* disable interrupts */
    do {
    /* loop as long as the CAN controller shows interrupts */

    /*========== receive interrupt */
    if( irqsrc & (IFLAG_BUFnM(RECEIVE_STD_OBJ)|IFLAG_BUFnM(RECEIVE_EXT_OBJ))) {
	/* when we got an interrupt and after holding the MB we are 
           safe to read the data in the MB; Donot poll the Control/status word
         */
	unsigned oid;
	int index;
    	DBGprint(DBG_DATA, (" => got  RX IRQ[%d]: 0x%0x\n", board, irqsrc));
    	if (irqsrc & (IFLAG_BUFnM(RECEIVE_STD_OBJ))) {
		index= RECEIVE_STD_OBJ;
		oid  = MCF_FLEXCAN_MB_ID(board,RECEIVE_STD_OBJ) >> 18;
        } else {
		index= RECEIVE_EXT_OBJ;
		oid  = MCF_FLEXCAN_MB_ID(board,RECEIVE_EXT_OBJ);
    	}

	ctrl = MCF_FLEXCAN_MB_CNT(board,index);

	/* clear the flag after MB is held */
	MCF_FLEXCAN_IFLAG(board) = IFLAG_BUFnM(index);

      if(Can_read_flag ) {
	if (oid == last_tx_id && TxFifo->active) {
           /*
	    This is obviously a message we sent ourselves recently.
	    There is no point in delivering this to the upper layer.
	    We rely on the fact
	    that no other node will ever send a message with the same OID.
            */
            if (selfreception) {
               (RxFifo->data[RxFifo->head]).flags |= MSG_SELF;
            }
        }
	if(!TxFifo->active || oid != last_tx_id || selfreception)
        {
	/* get message length,  strip length code */
	dummy = (unsigned int)((ctrl & MB_CNT_LENGTH(0xF))>>16);
	if (dummy > 8) dummy = 8;	/* limit count to 8 bytes */
	(RxFifo->data[RxFifo->head]).length = dummy;
	memset(&(RxFifo->data[RxFifo->head].data[0]), 0, 8*(sizeof(unsigned char)));
	if (irqsrc & (IFLAG_BUFnM(RECEIVE_STD_OBJ)))
	{
		(RxFifo->data[RxFifo->head]).id =  (MCF_FLEXCAN_MB_ID(board,RECEIVE_STD_OBJ) >> 18);
	} else {
		(RxFifo->data[RxFifo->head]).id =  MCF_FLEXCAN_MB_ID(board,RECEIVE_EXT_OBJ);
		(RxFifo->data[RxFifo->head]).flags = MSG_EXT;
	}
	memcpy((void*)&(RxFifo->data[RxFifo->head].data[0]),(void*) &(MCF_FLEXCAN_MB_DB(board,index, 0)), dummy);
        /* safe to release the MB */
	MCF_FLEXCAN_TIMER(board);

        do_gettimeofday(&(RxFifo->data[RxFifo->head]).timestamp);
        /* preset flags */
        (RxFifo->data[RxFifo->head]).flags =
        		(RxFifo->status & BUF_OVERRUN ? MSG_BOVR : 0);
	RxFifo->status = BUF_OK;

	RxFifo->head = ++(RxFifo->head) % MAX_BUFSIZE;
	if(RxFifo->head == RxFifo->tail) {
	    printk("CAN[%d] RX: FIFO overrun\n", board);
	    RxFifo->status = BUF_OVERRUN;
	}
	/* This function will wake up all processes
	   that are waiting on this event queue,
	   that are in interruptible sleep
	 */
	wake_up_interruptible(  &CanWait[board] );
	} else
	MCF_FLEXCAN_TIMER(board);

      } else
	MCF_FLEXCAN_TIMER(board);
    } 

    if( irqsrc & IFLAG_BUFnM(RECEIVE_RTR_OBJ))
	{
	DBGprint(DBG_DATA, (" => got  TX IRQ[%d]: 0x%0x\n", board, irqsrc));
	while ((ctrl = MCF_FLEXCAN_MB_CNT(board,RECEIVE_RTR_OBJ)
	       & (REC_CODE_BUSY << 24)) == (REC_CODE_BUSY << 24)) {
        	/* printk("CAN_int, tx REC_CODE_BUSY"); */
	;
	}
	while ((ctrl = MCF_FLEXCAN_MB_CNT(board,RECEIVE_RTR_OBJ)
	       & (TRANS_CODE_TRANSMIT_ONCE_RTR_ALWAYS << 24)) == (TRANS_CODE_TRANSMIT_ONCE_RTR_ALWAYS << 24)) {
        	/* printk("CAN_int, tx REC_CODE_BUSY"); */
	;
	}
	dummy = (unsigned int)((ctrl & MB_CNT_LENGTH(0xF))>>16);
	if (dummy > 8) dummy = 8;	/* limit count to 8 bytes */
	(RxFifo->data[RxFifo->head]).length = dummy;
	memset(&(RxFifo->data[RxFifo->head].data[0]), 0, 8*(sizeof(unsigned char)));
	if (ctrl & (MB_CNT_IDE))
	{
		(RxFifo->data[RxFifo->head]).id =  MCF_FLEXCAN_MB_ID(board,RECEIVE_RTR_OBJ);
		(RxFifo->data[RxFifo->head]).flags = MSG_EXT | MSG_RTR;
	} else {
		(RxFifo->data[RxFifo->head]).id =  (MCF_FLEXCAN_MB_ID(board,RECEIVE_RTR_OBJ) >> 18);
		(RxFifo->data[RxFifo->head]).flags = MSG_RTR;
	}
    	RxFifo->status = BUF_OK;
	RxFifo->head = ++(RxFifo->head) % MAX_BUFSIZE;
	if(RxFifo->head == RxFifo->tail) {
	    printk("CAN[%d] RX: FIFO overrun\n", board);
	    RxFifo->status = BUF_OVERRUN;
	}
	MCF_FLEXCAN_IFLAG(board) = IFLAG_BUFnM(RECEIVE_RTR_OBJ);
	MCF_FLEXCAN_TIMER(board);
        }


    if (irqsrc & (IFLAG_BUFnM(TRANSMIT_OBJ)))
    {
	/*========== transmit interrupt */
	DBGprint(DBG_DATA, (" => got  TX IRQ[%d]: 0x%0x\n", board, irqsrc));
	if(TxFifo->free[TxFifo->tail] == BUF_EMPTY) {
	    TxFifo->status = BUF_EMPTY;
            TxFifo->active = 0;
	    ctrl = MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ);
	    if ( (MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) & (REC_CODE_FULL << 24)) == (REC_CODE_FULL << 24) )
	    {
	    dummy = (unsigned int)((ctrl & MB_CNT_LENGTH(0xF))>>16);
	    if (dummy > 8) dummy = 8;	/* limit count to 8 bytes */
	    (RxFifo->data[RxFifo->head]).length = dummy;
	    memset(&(RxFifo->data[RxFifo->head].data[0]), 0, 8*(sizeof(unsigned char)));
	    (RxFifo->data[RxFifo->head]).id =  (MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ) >> 18);
	    memcpy((void*)&(RxFifo->data[RxFifo->head].data[0]),(void*) &(MCF_FLEXCAN_MB_DB(board,TRANSMIT_OBJ, 0)), dummy);
	    RxFifo->status = BUF_OK;
	    RxFifo->head = ++(RxFifo->head) % MAX_BUFSIZE;
	    if(RxFifo->head == RxFifo->tail) {
	    printk("CAN[%d] RX: FIFO overrun\n", board);
	    RxFifo->status = BUF_OVERRUN;
		}
	    /* This function will wake up all processes
	       that are waiting on this event queue,
	       that are in interruptible sleep
	    */
	    wake_up_interruptible(  &CanWait[board] );
	    }
	    /* Reset Interrupt pending at Transmit Object */
  	    MCF_FLEXCAN_IFLAG(board) = IFLAG_BUFnM(TRANSMIT_OBJ);
	    MCF_FLEXCAN_TIMER(board);
	    break; /* no more to send */
	} else {

	    local_irq_save(flags);
	    {
	    /* Real Transmit Interrupt */
	    /* Writing Control/Status word to hold TX Message Object inactive */
	    unsigned int tx_code;
	    tx_code = MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) & ~(MB_CNT_CODE(0x0F));
	    MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = tx_code | MB_CNT_CODE(TRANS_CODE_NOT_READY);
	    	if( (TxFifo->data[TxFifo->tail]).flags & MSG_EXT ) {
			/* use ID in extended message format */
			DBGprint(DBG_DATA, ("---> send ext message \n"));
			if( (TxFifo->data[TxFifo->tail]).flags & MSG_RTR ) {
			MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = (
				MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ)
                                | MB_CNT_IDE
				| MB_CNT_RTR
				| MB_CNT_SRR);
			MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ) = MB_ID_EXT(TxFifo->data[TxFifo->tail].id);
			} else {
			MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = (
				MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ)
                                | MB_CNT_IDE);
			MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ) = MB_ID_EXT(TxFifo->data[TxFifo->tail].id);
			}
			last_tx_id = MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ);
		} else {
			DBGprint(DBG_DATA, ("---> send std message \n"));
			if( (TxFifo->data[TxFifo->tail]).flags & MSG_RTR ) {
			MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = (
				MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ)
                                | MB_CNT_RTR);
       			MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ) = MB_ID_STD(TxFifo->data[TxFifo->tail].id);
			} else {
			MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = ( MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ));
        		MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ) = MB_ID_STD(TxFifo->data[TxFifo->tail].id);
			}
			last_tx_id = MCF_FLEXCAN_MB_ID(board,TRANSMIT_OBJ) >> 18;
		}
		dummy = (TxFifo->data[TxFifo->tail]).length;
                for( i = 0; i < dummy; i++) {
                    CAN_OBJ[TRANSMIT_OBJ].msg[i] =
                        TxFifo->data[TxFifo->tail].data[i];
                }

		/* Writing Control/Status word (active code, length) */
	        MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) &= ~ (MB_CNT_LENGTH(0x0f));
		MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ) = (
			MCF_FLEXCAN_MB_CNT(board,TRANSMIT_OBJ)
			| MB_CNT_CODE(TRANS_CODE_TRANSMIT_ONCE)
			| MB_CNT_LENGTH(dummy));
		TxFifo->free[TxFifo->tail] = BUF_EMPTY; /* now this entry is EMPTY */
		TxFifo->tail = ++(TxFifo->tail) % MAX_BUFSIZE;
	    }
	    local_irq_restore(flags);
	    /* Reset Interrupt pending at Transmit Object */
  	    MCF_FLEXCAN_IFLAG(board) = IFLAG_BUFnM(TRANSMIT_OBJ);
	}
    }

    } while( (irqsrc = MCF_FLEXCAN_IFLAG(board)) != 0);

    MCF_FLEXCAN_IMASK(board) = mask; /* enable interrupts */

/* IRQdone: */
    DBGprint(DBG_DATA, (" => leave IRQ[%d]\n", board));
	return IRQ_HANDLED;
}

void CAN_register_dump(int board)
{

#if DEBUG

#if defined(CONFIG_FIRE_ENGINE)
    volatile flex_can_t *tou_can = (flex_can_t *)(MCF_MBAR + 0xA000 + board*(0x0800));
#elif defined(CONFIG_M532x) || defined(CONFIG_M5253)
    volatile flex_can_t *tou_can = (flex_can_t *)(MCF_FLEXCAN_BASEADDR(board));
#endif

    printk("Flex CAN register layout\n");

#define  printregister(s, name) printk(s, &name , name)

    printregister(" CAN_ModulConfigRegister      %p %0lx\n", CAN_ModulConfigRegister);
    printregister(" CAN_ControlReg               %p %0lx\n", CAN_ControlReg);

    printregister(" CAN_TimerRegister            %p %0lx\n", CAN_TimerRegister);
    printregister(" CAN_ReceiveGlobalMask        %p %0lx\n", CAN_ReceiveGlobalMask);
    printregister(" CAN_ReceiveBuffer14Mask      %p %0lx\n", CAN_ReceiveBuffer14Mask);
    printregister(" CAN_ReceiveBuffer15Mask      %p %0lx\n", CAN_ReceiveBuffer15Mask);
    printregister(" CAN_ErrorCounterRegister     %p %0lx\n", CAN_ErrorCounterRegister);
    printregister(" CAN_ErrorStatusRegister      %p %0lx\n", CAN_ErrorStatusRegister);
    printregister(" CAN_InterruptMasks           %p %0lx\n", CAN_InterruptMasks);
    printregister(" CAN_InterruptFlags           %p %0lx\n", CAN_InterruptFlags);

#endif

}

void CAN_object_dump(int object)
{
    int board = 0;   /* be prepared for an board paramter, if later on .. */
    unsigned long id;

#if defined(CONFIG_M532x) || defined(CONFIG_M5253)
    volatile unsigned char *cp = (unsigned char *)(MCF_FLEXCAN_BASEADDR(CAN_MODULE) + (0x10 * object));
#else
	/* For 547x/8x */
#endif
    for(id = 0; id < 16; id++) {
	printk("%2x ", *(cp + id));
    }
    printk("\n");

    printk("Flex CAN object %d\n", object);
    id = CAN_OBJ[object].ctl_status;
    printk(" Ctrl/Status 0x%lx\n", id);
    id = CAN_OBJ[object].id.ext;
    printk(" Id 0x%lx\n", id);
    
}
