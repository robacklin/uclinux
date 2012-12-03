/***************************************************************************/

/*
 *	linux/arch/m68knommu/platform/532x/config.c
 *
 *	Copyright (C) 1999-2002, Greg Ungerer (gerg@snapgear.com)
 *	Copyright (C) 2000, Lineo (www.lineo.com)
 *	Yaroslav Vinogradov yaroslav.vinogradov@freescale.com
 *	Copyright Freescale Semiconductor, Inc 2006
 *	Copyright (c) 2006, emlix, Sebastian Hess <sh@emlix.com>
 *	Copyright (c) 2009  Arcturus Networks Inc. <www.ArcturusNetworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/***************************************************************************/

#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/init.h>
#include <linux/io.h>
#include <asm/machdep.h>
#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include <asm/mcfuart.h>
#include <asm/mcfdma.h>
#include <asm/mcfwdebug.h>

/***************************************************************************/

#if IS_ENABLED(CONFIG_SPI_COLDFIRE_QSPI)

static void __init m532x_qspi_init(void)
{
	/* setup QSPS pins for QSPI with gpio CS control */
	writew(0x01f0, MCF_GPIO_PAR_QSPI);
}

#endif /* IS_ENABLED(CONFIG_SPI_COLDFIRE_QSPI) */

/***************************************************************************/

static void __init m532x_uarts_init(void)
{
	/* UART GPIO initialization */
	MCF_GPIO_PAR_UART |= 0x0FFF;
}

/***************************************************************************/

static void __init m532x_fec_init(void)
{
	/* Set multi-function pins to ethernet mode for fec0 */
	MCF_GPIO_PAR_FECI2C |= (MCF_GPIO_PAR_FECI2C_PAR_MDC_EMDC |
		MCF_GPIO_PAR_FECI2C_PAR_MDIO_EMDIO);
	MCF_GPIO_PAR_FEC = (MCF_GPIO_PAR_FEC_PAR_FEC_7W_FEC |
		MCF_GPIO_PAR_FEC_PAR_FEC_MII_FEC);
}

/***************************************************************************/

/*
 * mcf532x_panic_blink() will flash the uCsimm matherboard LEDs and is
 * called when kernel panics. Flashing LEDs is useful for users who may
 * not see the console.
 */

#if defined(CONFIG_UC53281EVM)
static long mcf532x_panic_blink(long count)
{
	leds_event(led_panic);
	return 999;
}
#endif

/***************************************************************************/

void __init config_BSP(char *commandp, int size)
{
#if !defined(CONFIG_BOOTPARAM)
	/* Copy command line from FLASH to local buffer... */
	memcpy(commandp, (char *) 0x4000, 4);
	if(strncmp(commandp, "kcl ", 4) == 0){
		memcpy(commandp, (char *) 0x4004, size);
		commandp[size-1] = 0;
	} else {
		memset(commandp, 0, size);
	}
#endif

	mach_sched_init = hw_timer_init;
	m532x_uarts_init();
	m532x_fec_init();
#if IS_ENABLED(CONFIG_SPI_COLDFIRE_QSPI)
	m532x_qspi_init();
#endif

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
/* Board initialization */
/***************************************************************************/

/*
 *  SDRAM Timing Parameters
 */  
#define SDRAM_BL	8	/* # of beats in a burst */
#define SDRAM_TWR	2	/* in clocks */
#define SDRAM_CASL	2.5	/* CASL in clocks */
#define SDRAM_TRCD	2	/* in clocks */
#define SDRAM_TRP	2	/* in clocks */
#define SDRAM_TRFC	7	/* in clocks */
#define SDRAM_TREFI	7800	/* in ns */
#define SYSTEM_PERIOD	12.5

#define FLASH_ADDRESS  (0x00000000)
#define SDRAM_ADDRESS  (0x40000000)

void wtm_init(void)
{
	/* Disable watchdog timer */
	MCF_WTM_WCR = 0;
}

#define MCF_SCM_BCR_GBW		(0x00000100)
#define MCF_SCM_BCR_GBR		(0x00000200)

void scm_init(void)
{
	/* All masters are trusted */
	MCF_SCM_MPR = 0x77777777;
    
	/* Allow supervisor/user, read/write, and trusted/untrusted
	   access to all slaves */
	MCF_SCM_PACRA = 0;
	MCF_SCM_PACRB = 0;
	MCF_SCM_PACRC = 0;
	MCF_SCM_PACRD = 0;
	MCF_SCM_PACRE = 0;
	MCF_SCM_PACRF = 0;

	/* Enable bursts */
	MCF_SCM_BCR = (MCF_SCM_BCR_GBR | MCF_SCM_BCR_GBW);
}


void fbcs_init(void)
{
#if defined(CONFIG_COBRA5329)
	/* The COBRA5329 by senTec needs this settings */

        /* Boot Flash connected to FBCS0 */
        MCF_FBCS0_CSAR = FLASH_ADDRESS;
        MCF_FBCS0_CSCR = (MCF_FBCS_CSCR_PS_16
                        | MCF_FBCS_CSCR_BEM
                        | MCF_FBCS_CSCR_AA
                        | MCF_FBCS_CSCR_WS(8));

        MCF_FBCS0_CSMR = (MCF_FBCS_CSMR_BAM_1G
                        | MCF_FBCS_CSMR_V);

        /* Fix bug #10 in the errata */
        MCF_FBCS1_CSAR = 0xC0000000;
        MCF_FBCS1_CSCR = (MCF_FBCS_CSCR_PS_16
                        | MCF_FBCS_CSCR_BEM
                        | MCF_FBCS_CSCR_AA
                        | MCF_FBCS_CSCR_WS(8));

        MCF_FBCS1_CSMR = (0x30000000
                        | MCF_FBCS_CSMR_V
                        | MCF_FBCS_CSMR_WP );
#else
	MCF_GPIO_PAR_CS = 0x0000003E;

	/* Latch chip select */
	MCF_FBCS1_CSAR = 0x10080000;

	MCF_FBCS1_CSCR = 0x002A3580 | (MCF_FBCS1_CSCR&0x200);
	MCF_FBCS1_CSMR = (MCF_FBCS_CSMR_BAM_2M | MCF_FBCS_CSMR_V);

	/* Initialize latch to drive signals to inactive states */
	*((u16 *)(0x10080000)) = 0xD3FF;

	/* Boot Flash connected to FBCS0 */
	MCF_FBCS0_CSAR = FLASH_ADDRESS;
	MCF_FBCS0_CSCR = (MCF_FBCS_CSCR_PS_16
			| MCF_FBCS_CSCR_BEM
			| MCF_FBCS_CSCR_AA
			| MCF_FBCS_CSCR_SBM
			| MCF_FBCS_CSCR_WS(7));
	MCF_FBCS0_CSMR = (MCF_FBCS_CSMR_BAM_32M
			| MCF_FBCS_CSMR_V);
#endif
}

void sdramc_init(void)
{
	/*
	 * Check to see if the SDRAM has already been initialized
	 * by a run control tool
	 */
	if (!(MCF_SDRAMC_SDCR & MCF_SDRAMC_SDCR_REF)) {
		/* SDRAM chip select initialization */
		
		/* Initialize SDRAM chip select */
		MCF_SDRAMC_SDCS0 = (0
			| MCF_SDRAMC_SDCS_BA(SDRAM_ADDRESS)
			| MCF_SDRAMC_SDCS_CSSZ(MCF_SDRAMC_SDCS_CSSZ_32MBYTE));

	/*
	 * Basic configuration and initialization
	 */
	MCF_SDRAMC_SDCFG1 = (0
		| MCF_SDRAMC_SDCFG1_SRD2RW((int)((SDRAM_CASL + 2) + 0.5 ))
		| MCF_SDRAMC_SDCFG1_SWT2RD(SDRAM_TWR + 1)
		| MCF_SDRAMC_SDCFG1_RDLAT((int)((SDRAM_CASL*2) + 2))
		| MCF_SDRAMC_SDCFG1_ACT2RW((int)((SDRAM_TRCD ) + 0.5))
		| MCF_SDRAMC_SDCFG1_PRE2ACT((int)((SDRAM_TRP ) + 0.5))
		| MCF_SDRAMC_SDCFG1_REF2ACT((int)(((SDRAM_TRFC) ) + 0.5))
		| MCF_SDRAMC_SDCFG1_WTLAT(3));
	MCF_SDRAMC_SDCFG2 = (0
		| MCF_SDRAMC_SDCFG2_BRD2PRE(SDRAM_BL/2 + 1)
		| MCF_SDRAMC_SDCFG2_BWT2RW(SDRAM_BL/2 + SDRAM_TWR)
		| MCF_SDRAMC_SDCFG2_BRD2WT((int)((SDRAM_CASL+SDRAM_BL/2-1.0)+0.5))
		| MCF_SDRAMC_SDCFG2_BL(SDRAM_BL-1));

            
	/*
	 * Precharge and enable write to SDMR
	 */
        MCF_SDRAMC_SDCR = (0
		| MCF_SDRAMC_SDCR_MODE_EN
		| MCF_SDRAMC_SDCR_CKE
		| MCF_SDRAMC_SDCR_DDR
		| MCF_SDRAMC_SDCR_MUX(1)
		| MCF_SDRAMC_SDCR_RCNT((int)(((SDRAM_TREFI/(SYSTEM_PERIOD*64)) - 1) + 0.5))
		| MCF_SDRAMC_SDCR_PS_16
		| MCF_SDRAMC_SDCR_IPALL);            

	/*
	 * Write extended mode register
	 */
	MCF_SDRAMC_SDMR = (0
		| MCF_SDRAMC_SDMR_BNKAD_LEMR
		| MCF_SDRAMC_SDMR_AD(0x0)
		| MCF_SDRAMC_SDMR_CMD);

	/*
	 * Write mode register and reset DLL
	 */
	MCF_SDRAMC_SDMR = (0
		| MCF_SDRAMC_SDMR_BNKAD_LMR
		| MCF_SDRAMC_SDMR_AD(0x163)
		| MCF_SDRAMC_SDMR_CMD);

	/*
	 * Execute a PALL command
	 */
	MCF_SDRAMC_SDCR |= MCF_SDRAMC_SDCR_IPALL;

	/*
	 * Perform two REF cycles
	 */
	MCF_SDRAMC_SDCR |= MCF_SDRAMC_SDCR_IREF;
	MCF_SDRAMC_SDCR |= MCF_SDRAMC_SDCR_IREF;

	/*
	 * Write mode register and clear reset DLL
	 */
	MCF_SDRAMC_SDMR = (0
		| MCF_SDRAMC_SDMR_BNKAD_LMR
		| MCF_SDRAMC_SDMR_AD(0x063)
		| MCF_SDRAMC_SDMR_CMD);
				
	/*
	 * Enable auto refresh and lock SDMR
	 */
	MCF_SDRAMC_SDCR &= ~MCF_SDRAMC_SDCR_MODE_EN;
	MCF_SDRAMC_SDCR |= (0
		| MCF_SDRAMC_SDCR_REF
		| MCF_SDRAMC_SDCR_DQS_OE(0xC));
	}
}

void gpio_init(void)
{
	/* Enable UART0 pins */
	MCF_GPIO_PAR_UART = ( 0
		| MCF_GPIO_PAR_UART_PAR_URXD0
		| MCF_GPIO_PAR_UART_PAR_UTXD0);

	/* Initialize TIN3 as a GPIO output to enable the write
	   half of the latch */
	MCF_GPIO_PAR_TIMER = 0x00;
	__raw_writeb(0x08, MCFGPIO_PDDR_TIMER);
	__raw_writeb(0x00, MCFGPIO_PCLRR_TIMER);

}

asmlinkage void __init sysinit(void)
{
	wtm_init();
	scm_init();
	gpio_init();
	fbcs_init();
	sdramc_init();
}

