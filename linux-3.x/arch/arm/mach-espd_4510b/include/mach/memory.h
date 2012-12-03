/*
 * linux/include/asm-armnommu/arch-espd_4510b/memory.h
 *
 * Copyright (C) 2003 SAMSUNG ELECTRONICS 
 *		Hyok S. Choi <hyok.choi@samsung.com>
 */

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#define TASK_SIZE	(0x01a00000UL)
#define TASK_SIZE_26	TASK_SIZE

#define PHYS_OFFSET	(CONFIG_DRAM_BASE)
#define PAGE_OFFSET 	(PHYS_OFFSET)
#define END_MEM     	(CONFIG_DRAM_BASE + CONFIG_DRAM_SIZE)

#define	__arch_dma_to_virt(dev, x)	((void *) __phys_to_virt(x))
#define	__arch_virt_to_dma(dev, x)	((dma_addr_t) __virt_to_phys(x))

#define	__arch_pfn_to_dma(dev, pfn)	((dma_addr_t ) __pfn_to_phys(pfn))
#define	__arch_dma_to_pfn(dev, x)	__phys_to_pfn(x)

#endif /*  __ASM_ARCH_MEMORY_H */
