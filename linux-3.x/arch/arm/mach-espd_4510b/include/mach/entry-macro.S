/*
 *  arch/armnommu/mach-espd_4510b/entry-macro.S
 *
 *  Copyright (c) 2004	Cucy Systems (http://www.cucy.com)
 *  Curt Brune <curt@cucy.com>
 *
 *  defines machine dependent entry macros.
 *  included in the arch/armnommu/kernel/entry.S
 *  
 */
#include <mach/irqs.h>
#include <mach/hardware.h>

		.macro disable_fiq
		.endm

		.macro get_irqnr_preamble, base, tmp
		.endm

		.macro arch_ret_to_user, tmp1, tmp2
		.endm

		.macro get_irqnr_and_base, irqnr, irqstat, base, tmp
		ldr	\base, =REG_INTOSET_IRQ
		ldr	\irqnr, [\base]
		mov	\irqnr, \irqnr, lsr #2
		teq	\irqnr, #NR_IRQS
		.endm

		.macro irq_prio_table
		.endm

