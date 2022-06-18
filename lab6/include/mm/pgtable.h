#ifndef _PGTABLE_H_
#define _PGTABLE_H_

#define PGD_SHIFT	    	(39)
#define PGD_TYPE_MASK		(3 << 0)
#define PGD_TYPE_TABLE		(3 << 0)
#define PGD_TABLE_BIT		(1 << 1)
#define PGD_TYPE_BLOCK		(1 << 0)

#define PUD_SHIFT	    	(30)
#define PUD_TYPE_MASK		(3 << 0)
#define PUD_TYPE_TABLE		(3 << 0)
#define PUD_TABLE_BIT		(1 << 1)
#define PUD_TYPE_BLOCK		(1 << 0)

#define PMD_SHIFT	    	(21)
#define PMD_TYPE_MASK		(3 << 0)
#define PMD_TYPE_TABLE		(3 << 0)
#define PMD_TABLE_BIT		(1 << 1)
#define PMD_TYPE_BLOCK		(1 << 0)

#define PTE_SHIFT	    	(12)
#define PTE_TYPE_MASK		(3 << 0)
#define PTE_TYPE_FAULT		(0 << 0)
#define PTE_TYPE_PAGE		(3 << 0)
#define PTE_TABLE_BIT		(1 << 1)

#define PAGE_MAIR_SHIFT         (2)
#define PAGE_ATTR_USER		    (1 << 6)	
#define PAGE_ATTR_RDONLY		(1 << 7)
#define PAGE_ATTR_SHARED		(3 << 8)
#define PAGE_ATTR_AF			(1 << 10)	
#define PAGE_ATTR_NG			(1 << 11)	
#define PAGE_ATTR_DBM			(1ul << 51)	
#define PAGE_ATTR_CONT		    (1ul << 52)	
#define PAGE_ATTR_PXN			(1ul << 53)	
#define PAGE_ATTR_UXN			(1ul << 54)
#define PAGE_ATTR_DIRTY			(1ul << 55) // Hardware doesn't support dirty bit. 

#define BLOCK_MAIR_SHIFT        (2)
#define BLOCK_ATTR_USER		    (1 << 6)	
#define BLOCK_ATTR_RDONLY		(1 << 7)
#define BLOCK_ATTR_SHARED		(3 << 8)
#define BLOCK_ATTR_AF			(1 << 10)	
#define BLOCK_ATTR_NG			(1 << 11)	
#define BLOCK_ATTR_DBM			(1ul << 51)	
#define BLOCK_ATTR_CONT		    (1ul << 52)	
#define BLOCK_ATTR_PXN			(1ul << 53)	
#define BLOCK_ATTR_UXN			(1ul << 54)	

#define PHYS_ADDR_MASK  (0xfffffffff000)

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

#define BOOT_PGD_ATTR (PGD_TYPE_TABLE)
#define BOOT_PUD_ATTR (PUD_TYPE_BLOCK | BLOCK_ATTR_AF | (MAIR_IDX_DEVICE_nGnRnE << BLOCK_MAIR_SHIFT))

#define VM_PGD_ATTR (PGD_TYPE_TABLE)
#define VM_PUD_ATTR (PUD_TYPE_TABLE)
#define VM_PMD_ATTR (PMD_TYPE_TABLE)
#define VM_PTE_DEVICE_ATTR (PTE_TYPE_PAGE | PAGE_ATTR_AF | (MAIR_IDX_DEVICE_nGnRnE << PAGE_MAIR_SHIFT))
#define VM_PTE_NORMAL_ATTR (PTE_TYPE_PAGE | PAGE_ATTR_AF | (MAIR_IDX_NORMAL_NOCACHE << PAGE_MAIR_SHIFT))

#define VM_PTE_USER_ATTR (PTE_TYPE_PAGE | PAGE_ATTR_AF | PAGE_ATTR_USER | (MAIR_IDX_NORMAL_NOCACHE << PAGE_MAIR_SHIFT))
#endif
