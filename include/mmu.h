#ifndef MMU_H
#define MMU_H
#include "esr.h"
#include "uart.h"
#include "list.h"
#include "sched.h"
#include "string.h"
#include "malloc.h"
#include "exception.h"

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) | (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1
#define MAIR_CONFIG_DEFAULT ((MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)))

#define PD_TABLE 0b11L
#define PD_BLOCK 0b01L
#define PD_UNX (1L << 54)
#define PD_KNX (1L << 53)
#define PD_ACCESS (1L << 10)
#define PD_UK_ACCESS (1L << 6)
#define PD_RDONLY    (1L << 7)
#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)

#define PHYS_TO_VIRT(x) (x + 0xffff000000000000)
#define VIRT_TO_PHYS(x) (x - 0xffff000000000000)
#define ENTRY_ADDR_MASK   0xfffffffff000L

#define kernel_pgd_addr 0x1000
#define kernel_pud_addr 0x2000

#define DATA_ABORT_LOWER 0b100100
#define INS_ABORT_LOWER 0b100000

#define TF_LEVEL0 0b000100
#define TF_LEVEL1 0b000101
#define TF_LEVEL2 0b000110
#define TF_LEVEL3 0b000111

void *set_2M_kernel_mmu(void *x0);
void map_one_page(unsigned long *pgd_p, unsigned long va, unsigned long pa, unsigned long flag);
void add_vma(struct thread *t, unsigned long va, unsigned long size, unsigned long pa, unsigned long rwx, int is_alloced);
void free_page_tables(unsigned long *page_table, int level);
void handle_abort(esr_el1_t* esr_el1);
void map_one_page_rwx(unsigned long *pgd_p, unsigned long va, unsigned long pa, unsigned long rwxflag);
void seg_fault();

typedef struct vm_area_struct {
    list_head_t listhead;
    unsigned long virt_addr;
    unsigned long phys_addr;
    unsigned long area_size;
    unsigned long rwx;      // r = 1, w = 2, x = 4
    int is_alloced;
} vm_area_struct_t;

#endif