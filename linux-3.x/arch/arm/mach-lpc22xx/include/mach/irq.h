/*
 *  arch/arm/mach-lpc22xx/include/mach/irq.h
 *
 *  Copyright (C) 2004 Philips Semiconductors
 */
#ifndef __LPC22xx_irq_h
#define __LPC22xx_irq_h

#include <asm/hardware.h>
#include <asm/io.h>
#include <mach/irq.h>
#include <mach/irqs.h>

extern unsigned  int fixup_irq(int i);

extern void do_IRQ(int irq, struct pt_regs *regs);

extern void lpc22xx_init_irq(void);

#define 	irq_init_irq 	lpc22xx_init_irq

#endif 
