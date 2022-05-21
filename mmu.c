#include "mmu.h"


void page_table_alloc(unsigned long table, unsigned long next, unsigned long attribute, unsigned int offset) {
    asm volatile("str %0, [%1]\n"::"r"(next|attribute),"r"(table + offset*8));
}

void block_alloc(unsigned long table, unsigned long address, unsigned long attribute, unsigned int offset) {
    asm volatile("str %0, [%1]\n"::"r"(address|attribute),"r"(table + offset*8));
}

void video_paging() {
    // paging 0x3C000000 ~ 0x3CFFFFFF
    page_table_alloc(0x5000, 0x6000, BOOT_PGD_ATTR, 0);
    page_table_alloc(0x6000, 0x7000, BOOT_PUD_ATTR, 0);
    for (int i=0; i<24; i++) {
        block_alloc(0x7000, 0x3C000000 + 0x200000*(i), USER_READ_WRITE, 480+i);
    }
    asm volatile("msr ttbr0_el1, %0	\n"::"r"(0x5000));
}



