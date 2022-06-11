#include "mmu.h"
#include "memory.h"

extern uint64 *get_pgd();

void set_PGD() {
    uint64 *pgd_table = (uint64 *)PGD_BASE;
    
    pgd_table[0] = PUD_BASE | BOOT_PGD_ATTR;
}

// two-level translation (1G)
void set_PUD() {
    uint64 *pud_table = (uint64 *)PUD_BASE;

    pud_table[0] = PMD_BASE | BOOT_PUD_ATTR;
    pud_table[1] = (PMD_BASE + 0x1000L) | BOOT_PUD_ATTR;
}

// three-level translation (2MB) - only first one
void set_PMD() {
    uint64 *pmd_table = (uint64 *)PMD_BASE;

    for (int i = 0; i < 512 * 2; i++) { // 1G = 512 * 2M
        pmd_table[i] = (PTE_BASE + 0x1000L * i) | BOOT_PMD_ATTR;
    }
}

// four-level translation (4KB)
void set_PTE() {
    uint64 *pte_table = (uint64 *)PTE_BASE;

    for (int i = 0; i < 512 * 512 * 2; i++) { // 1G = 512 * 2M, 2M = 512 * 4K
        uint64 addr = 0x1000L * i;
        if(addr < PERIPHERAL_BASE) {
            pte_table[i] = addr | BOOT_PTE_NORMAL_NOCACHE_ATTR;
        }
        else {
            pte_table[i] = addr | BOOT_PTE_DEVICE_nGnRnE_ATTR;
        }
    }
}

void init_page_table(uint64 **table, int level) {
    lock_interrupt();
    *table = (uint64*)malloc(0x1000);
    memset(*table, 0, 0x1000);
    // convert to physical address for mmu
    *table = (uint64*)(VIR_TO_PHY((uint64)*table));
    unlock_interrupt();
}

// static uint64 ttcou = 0;
void map_page_table(thread_t *thread, uint64 *pgd, uint64 vir_addr, uint64 phy_addr, bool readOnly) {
    int page_idx[] = {
        (vir_addr >> 39) & 0x1ff, (vir_addr >> 30) & 0x1ff,
        (vir_addr >> 21) & 0x1ff, (vir_addr >> 12) & 0x1ff
    };

    // if(phy_addr >= 0x3D1FF000 && phy_addr < 0x3D203000) {
    //     uart_printf("PGD: 0x%x, Virtual Address: 0x%x, Physical Address: 0x%x\n", pgd, vir_addr, phy_addr);
    //     for(int i = 0; i < 4; i++) {
    //         uart_printf("Index-%d: %x\n", i, page_idx[i]);
    //     }
    // }

    uint64 *table = (uint64*)PHY_TO_VIR((uint64)pgd);
    for(int level = 0; level < 3; level++) {
        if(table[page_idx[level]] == 0L) {
            init_page_table((uint64**)&table[page_idx[level]], level + 1);
            table[page_idx[level]] |= PD_TABLE;
            // uart_printf("Create Table(0x%x, %d, 0x%x) at level %d: 0x%x\n", table, page_idx[level], phy_addr, level, table[page_idx[level]]);
            // ttcou++;
        }

        table = (uint64*)PHY_TO_VIR((table[page_idx[level]] & ~0xfffL));
        // if(phy_addr >= 0x3D1FF000 && phy_addr < 0x3D203000) {
        //     uart_printf("Next Table(0x%x, %d, 0x%x) at level %d: 0x%x\n", table, page_idx[level], phy_addr, level, table[page_idx[level]]);
        // }
    }

    if(readOnly) {
        table[page_idx[3]] = PD_NE_EL1 | phy_addr | BOOT_PTE_NORMAL_NOCACHE_ATTR | PD_RDONLY | PD_UK_ACCESS;
    }
    else {
        table[page_idx[3]] = phy_addr | BOOT_PTE_NORMAL_NOCACHE_ATTR | PD_UK_ACCESS;
    }
    // uart_printf("Entry: 0x%x\n", table[page_idx[3]]);
}

void set_page_tables_for_thread(thread_t *thread) {
    // map program
    lock_interrupt();
    // ttcou = 0;
    // uart_printf("PGD at 0x%x\n", thread->context.pgd);
    // uart_printf("DATA MAP\n");
    for (uint64 size = 0; size < thread->datasize; size += 0x1000L) {
        uint64 vir_addr = USER_KERNEL_BASE + size;
        uint64 phy_addr = VIR_TO_PHY((uint64)thread->data + size);
        // uart_printf("0x%x to 0x%x\n", vir_addr, phy_addr);
        map_page_table(thread, thread->context.pgd, vir_addr , phy_addr, false);
    }    
    // map stack
    // uart_printf("STACK MAP\n");
    for (uint64 size = 0; size < THREAD_STACK_SIZE; size += 0x1000L) {
        uint64 vir_addr = USER_STACK_BASE + size;
        uint64 phy_addr = VIR_TO_PHY((uint64)thread->stackPtr + size);
        // uart_printf("0x%x to 0x%x\n", vir_addr, phy_addr);
        map_page_table(thread, thread->context.pgd, vir_addr , phy_addr, false);
    }
    // map peripheral
    // uart_printf("PERIPHERAL MAP\n");
    for (uint64 addr = PERIPHERAL_START; addr < PERIPHERAL_END; addr += 0x1000L) {
        // uart_printf("0x%x to 0x%x\n", addr, addr);
        map_page_table(thread, thread->context.pgd, addr , addr, false);
    }

    // uart_printf("Num of pages: %d\n", ttcou);
    unlock_interrupt();
}

void free_page_table(uint64 vir_addr) {
    void *temp = (void*)(PHY_TO_VIR(vir_addr) & ~0xfff);
    memset(temp, 0, 0x1000);
    free(temp);
}

void free_page_tables(uint64 *pgd, int level) {
    uint64 *table = (uint64*)(PHY_TO_VIR((uint64)pgd) & ~0xfff);

    for(int idx = 0; idx <= 0x1ff; idx++) {  
        uint64 entry = table[idx];
        if(entry & PD_TABLE) {
            if(level < 2) {
                free_page_tables((uint64*)(entry & ~0xfff), level + 1);          
            }
            free_page_table(entry);
            // uart_printf("Free Table at level %d: 0x%x\n", level, entry & ~0xfff);
        }
    }        
}

void free_page_tables_for_thread(thread_t *thread) {
    lock_interrupt();
    free_page_tables(thread->context.pgd, 0);
    free_page_table((uint64)thread->context.pgd);
    unlock_interrupt();
}

void parser_table(uint64 vir_addr) {
    uint64 *pgd = get_pgd();
    uint64 *table = (uint64*)PHY_TO_VIR((uint64)pgd);
    int page_idx[] = {
        (vir_addr >> 39) & 0x1ff, (vir_addr >> 30) & 0x1ff,
        (vir_addr >> 21) & 0x1ff, (vir_addr >> 12) & 0x1ff
    };

    for(int level = 0; level < 4; level++) {
        uart_printf("Table entry(%d) at level %d: 0x%x\n", page_idx[level], level, table[page_idx[level]]);
        table = (uint64*)PHY_TO_VIR((table[page_idx[level]] & ~0xfff));
    }
}

void set_vm_list(list_head_t *list, uint64 vir_addr, uint64 phy_addr, uint64 size) {
    vm_cell_t *program_vm_cell = (vm_cell_t *)malloc(sizeof(vm_cell_t));
    program_vm_cell->vir_addr = vir_addr;
    program_vm_cell->phy_addr = phy_addr;
    program_vm_cell->size = (size % 0x1000) == 0 ? size : size + 0x1000 - (size % 0x1000);
    // uart_printf("add vm(0x%x): 0x%x => 0x%x, size = %d\n", program_vm_cell, vir_addr, phy_addr, program_vm_cell->size);
    list_add((list_head_t*)program_vm_cell, list);
}

void set_vm_list_for_thread(thread_t *thread) {
    lock_interrupt();
    
    set_vm_list(&thread->used_vm, USER_KERNEL_BASE, VIR_TO_PHY((uint64)thread->data), thread->datasize); // set program
    set_vm_list(&thread->used_vm, USER_STACK_BASE, VIR_TO_PHY((uint64)thread->stackPtr), THREAD_STACK_SIZE); // set stack
    set_vm_list(&thread->used_vm, PERIPHERAL_START, PERIPHERAL_START, PERIPHERAL_END - PERIPHERAL_START); // set peripheral

    unlock_interrupt();
}

void free_vm_list_for_thread(thread_t *thread) {
    list_head_t *ptr = thread->used_vm.next;
    while (ptr != &thread->used_vm) {
        list_head_t *next_ptr = ptr->next;
        // uart_printf("free vm(0x%x)\n", ptr);
        free(ptr);
        ptr = next_ptr;
    }
}

void handle_abort(esr_el1_t *esr_el1) {
    uint64 fail_addr;
    asm("mrs %0, FAR_EL1\n\t": "=r"(fail_addr));

    list_head_t *ptr;
    vm_cell_t *matched_ptr = NULL;
    list_for_each(ptr, &currThread->used_vm) {
        vm_cell_t *used_vm_cell = (vm_cell_t *)ptr;
        if (used_vm_cell->vir_addr <= fail_addr && (used_vm_cell->vir_addr + used_vm_cell->size) >= fail_addr) {
            matched_ptr = used_vm_cell;
            break;
        }
    }

    // not covered by this thread
    if (!matched_ptr) {
        seg_fault();
    }
    else {
        // Translation fault
        uint64 fail_level = esr_el1->iss & 0x3f;
        if (fail_level == TF_LEVEL0 || fail_level == TF_LEVEL1 || fail_level == TF_LEVEL2 || fail_level == TF_LEVEL3) {
            uart_printf("[Translation fault]: 0x%x\r\n", fail_addr);

            uint64 offset = (fail_addr - matched_ptr->vir_addr);
            uint64 aligned_offset = offset - (offset % 0x1000);
            map_page_table(currThread, currThread->context.pgd, matched_ptr->vir_addr + aligned_offset, matched_ptr->phy_addr + aligned_offset, false);
        }
        // Other Faults we don't handle
        else { 
            seg_fault();
        }
    }
}

void seg_fault() {
    uart_printf("[Segmentation fault]: Kill Process\r\n");
    thread_exit();
}
