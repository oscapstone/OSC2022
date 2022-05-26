#include "mmu.h"

#include "gpio.h"
#include "uart.h"
#include "mem.h"
#include "thread.h"
unsigned long get_high_pa(void *ptr) {
    return (unsigned long)ptr - KVA;
}

unsigned long get_low_pa(void *ptr) {
    unsigned long virtual_addr = (unsigned long)ptr;
    unsigned long *table = (unsigned long*)(store_pgd() + KVA);
    for (int i=3; i>=0; i--) {
        unsigned long offset = (virtual_addr >> (12 + i*9)) & 0x1FF;
        if ((table[offset] & 3) == 1) {
            table = (unsigned long *)((table[offset] & PD_ADDRESS_MASK) + KVA);
            break;
        }
        else if ((table[offset] & 3) == 3)
            table = (unsigned long *)((table[offset] & PD_ADDRESS_MASK) + KVA);
        else
            printf("Invalid entry\n");
    }
    return ((unsigned long)table | (virtual_addr & PAGE_OFFSET_MASK)) - KVA ;
}

void page_table_init(unsigned long* table) {
    int r=0; while(r++ < 512) table[r] = 0;
}

unsigned long* create_page_table() {
    unsigned long *table = kmalloc(4096);
    page_table_init(table);
    return table;
}

void page_table_alloc(unsigned long table, unsigned long next, unsigned long attribute, unsigned int offset) {
    unsigned long *alloc = (unsigned long*)table;
    alloc[offset] = next | attribute;
}

void block_alloc(unsigned long table, unsigned long address, unsigned long attribute, unsigned int offset) {
    unsigned long *alloc = (unsigned long*)table;
    alloc[offset] = address | attribute;
}

void* page_alloc(unsigned long pgd, unsigned long virtual_addr, unsigned long physical_addr, unsigned long attribute) {
    unsigned long *table = (unsigned long *)pgd;
    // PGD->PUD->PMD->PTE
    for (int i=3; i>0; i--) {
        unsigned long offset = (virtual_addr >> (12 + i*9)) & 0x1FF;
        if ((table[offset] & 1) == 0)
            table[offset] = get_high_pa(create_page_table()) | PD_TABLE;
        table = (unsigned long *)((table[offset] & PD_ADDRESS_MASK) + KVA);
    }
    // PTE->physical address
    unsigned long offset = (virtual_addr >> 12) & 0x1FF;
    if (physical_addr) {
        table[offset] = physical_addr | attribute | PD_PAGE;
        return (void*)(physical_addr + KVA);
    }
    else {
        void *kaddr = kmalloc(4096);
        table[offset] = get_high_pa(kaddr) | attribute | PD_PAGE;
        return kaddr;
    }
}

void page_free(unsigned long pgd, unsigned long virtual_addr) {
    unsigned long *table = (unsigned long*)(pgd + KVA);
    unsigned long *tmp;
    for (int i=3; i>=0; i--) {
        tmp = table;
        unsigned long offset = (virtual_addr >> (12 + i*9)) & 0x1FF;
        if ((table[offset] & 3) == 1) {
            table[offset] &= ~1;
            table = (unsigned long *)((table[offset] & PD_ADDRESS_MASK) + KVA);
            kfree(tmp);
            break;
        }
        else if ((table[offset] & 3) == 3) {
            table[offset] &= ~1;
            table = (unsigned long *)((table[offset] & PD_ADDRESS_MASK) + KVA);
            kfree(tmp);
        }
        else
            printf("Invalid entry\n");
    }
}

void change_attribute(unsigned long virtual_addr, unsigned long attribute) {
    unsigned long *table = (unsigned long*)(store_pgd() + KVA);
    for (int i=3; i>0; i--) {
        unsigned long offset = (virtual_addr >> (12 + i*9)) & 0x1FF;
        if ((table[offset] & 3) == 3)
            table = (unsigned long *)((table[offset] & PD_ADDRESS_MASK) + KVA);
        else
            printf("Invalid entry\n");
    }
    unsigned long offset = (virtual_addr >> 12) & 0x1FF;
    table[offset] = (table[offset] & PD_ADDRESS_MASK) | attribute | PD_PAGE;
}

void video_paging(unsigned long pgd, unsigned long pud, unsigned long pmd) {
    // paging 0x3C000000 ~ 0x3CFFFFFF
    page_table_init((unsigned long*)(pgd+KVA));
    page_table_alloc(pgd+KVA, pud, BOOT_PGD_ATTR, 0);
    page_table_init((unsigned long*)(pud+KVA));
    page_table_alloc(pud+KVA, pmd, BOOT_PUD_ATTR, 0);
    page_table_init((unsigned long*)(pmd+KVA));
    for (int i=0; i<24; i++) {
        block_alloc(pmd+KVA, 0x3C000000 + 0x200000*(i), USER_READ_WRITE, 480+i);
    }
}

void user_default_paging() {
    video_paging(0x5000, 0x6000, 0x7000);
    // block_alloc(0x7000+KVA, PHYSICAL_USER_PROGRAM, USER_READ_WRITE, 0);
}

void lower_data_abort_handler() {
	unsigned long far;
	asm volatile("mrs %0, FAR_EL1	\n":"=r"(far):);
	far = far & ~0xFFF;
    if (demand_find(thread_list.beg->demand, far)) {
	    printf("[Translation fault]: %x\n", far);
        thread_list.beg->user_stack = (char*)page_alloc(thread_list.beg->pgd + KVA, far, 0, USER_READ_WRITE);
    }
    else {
        printf("[Segmentation fault]: Kill Process\n");
	    // printf("data abort--%x\n", far);
        exit(0,0);
    }
}

unsigned long demand_log(unsigned long* list, unsigned long virtual_addr) {
    for (int i=0; i<16; i++) {
        if (list[i] == 0) {
            list[i] = virtual_addr;
            return i;
        }
    }
    printf("demand out of memory\n");
    while(1){}
    return 0;
}

int demand_find(unsigned long* list, unsigned long virtual_addr) {
    for (int i=0; i<16; i++) {
        if (list[i] == virtual_addr) {
            return 1;
        }
    }
    return 0;
}