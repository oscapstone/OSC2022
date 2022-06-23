#include "mmu.h"

// PGD's page frame at 0x1000
// PUD's page frame at 0x2000

// 2-level translation (1GB) => 3-level translation (2MB)
void* set_2M_kernel_mmu(void *x0) {
    // first entry (0x00000000 ~ 0x3FFFFFFF)
    unsigned long *pud_table_1 = (unsigned long *)0x3000;
    for (int i = 0; i < 512; i++) {
        // 0x3F000000 to 0x3FFFFFFF for peripherals
        if ((0x00000000 + (0x200000L) * i) >= 0x3F000000L)
            pud_table_1[i] = PD_ACCESS | PD_BLOCK | (0x00000000 + (0x200000L) * i) | (MAIR_DEVICE_nGnRnE << 2) | PD_UK_ACCESS | PD_UNX | PD_KNX;
        else
            pud_table_1[i] = PD_ACCESS | PD_BLOCK | (0x00000000 | (0x200000L) * i) | (MAIR_IDX_NORMAL_NOCACHE << 2);
    }
    // second entry (0x40000000 ~ 0x7FFFFFFF)
    unsigned long *pud_table_2 = (unsigned long *)0x4000;
    for (int i = 0; i < 512; i++)
        pud_table_2[i] = PD_ACCESS | PD_BLOCK | (0x40000000L | (0x200000L) * i) | (MAIR_IDX_NORMAL_NOCACHE << 2);
    // set PUD
    *(unsigned long *)(kernel_pud_addr) = PD_ACCESS | PD_TABLE | (unsigned long)pud_table_1;
    *(unsigned long *)(kernel_pud_addr + 0x8) = PD_ACCESS  | PD_TABLE | (unsigned long)pud_table_2;

    return x0;
}

void map_one_page(unsigned long *virt_pgd_p, unsigned long va, unsigned long pa, unsigned long flag) {
    unsigned long *table_p = virt_pgd_p;
    for (int level = 0; level < 4; level++) {
        // get index from VA
        unsigned int idx = (va >> (39 - level * 9)) & 0x1ff;
        // map page to table
        if (level == 3) {
            table_p[idx] = pa;
            table_p[idx] |= PD_ACCESS | PD_TABLE | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_KNX | flag;
            return;
        }
        // malloc for empty entry
        if (!table_p[idx]) {
            unsigned long* newtable_p = kmalloc(0x1000);
            memset(newtable_p, 0, 0x1000);
            table_p[idx] = VIRT_TO_PHYS((unsigned long)newtable_p);
            table_p[idx] |= PD_ACCESS | PD_TABLE | (MAIR_IDX_NORMAL_NOCACHE << 2);
        }
        // get next level entry
        table_p = (unsigned long*)PHYS_TO_VIRT((unsigned long)(table_p[idx] & ENTRY_ADDR_MASK));
    }
}

void add_vma(thread_t *t, unsigned long va, unsigned long size, unsigned long pa, unsigned long rwx, int is_alloced) {
    // alignment
    size = size % 0x1000 ? size + (0x1000 - size % 0x1000) : size;

    vm_area_struct_t *new_area = kmalloc(sizeof(vm_area_struct_t));
    new_area->rwx = rwx;
    new_area->area_size = size;
    new_area->virt_addr = va;
    new_area->phys_addr = pa;
    new_area->is_alloced = is_alloced;
    list_add_tail((list_head_t *)new_area, &t->vma_list);
}

void free_page_tables(unsigned long *page_table, int level) {
    unsigned long *table_virt = (unsigned long*)PHYS_TO_VIRT((char*)page_table);
    for (int i = 0; i < 512; i++) {
        if (table_virt[i] != 0) {
            unsigned long *next_table = (unsigned long*)(table_virt[i] & ENTRY_ADDR_MASK);
            if (table_virt[i] & PD_TABLE) {
                if (level!=2) {
                    free_page_tables(next_table, level + 1);
                }
                table_virt[i] = 0L;
                kfree(PHYS_TO_VIRT((char *)next_table));
            }
        }
    }
}

void handle_abort(esr_el1_t* esr_el1) {
    // get fault VA
    unsigned long long far_el1;
    asm volatile("mrs %0, FAR_EL1\n\t" : "=r"(far_el1));
    // check if far_el1 is in VMA
    list_head_t *pos;
    vm_area_struct_t *the_area_ptr = 0;
    list_for_each (pos, &curr_thread->vma_list) {
        if (((vm_area_struct_t *)pos)->virt_addr <= far_el1 && ((vm_area_struct_t *)pos)->virt_addr + ((vm_area_struct_t *)pos)->area_size >= far_el1) {
            the_area_ptr = (vm_area_struct_t *)pos;
            break;
        }
    }
    // far_el1 is not in VMA
    if (!the_area_ptr) {
        uart_printf("far_el1 is not in VMA\r\n");
        seg_fault();
    }
    // for translation fault
    if ((esr_el1->iss & 0x3f) == TF_LEVEL0 || (esr_el1->iss & 0x3f) == TF_LEVEL1 || (esr_el1->iss & 0x3f) == TF_LEVEL2 || (esr_el1->iss & 0x3f) == TF_LEVEL3) {
        uart_printf("[Translation fault]: 0x%x\r\n", far_el1);
        // offset align down 0x1000
        unsigned long addr_offset = (far_el1 - the_area_ptr->virt_addr);
        addr_offset = (addr_offset % 0x1000) == 0 ? addr_offset : addr_offset - (addr_offset % 0x1000);
        map_one_page_rwx(PHYS_TO_VIRT(curr_thread->context.ttbr0_el1), the_area_ptr->virt_addr + addr_offset, the_area_ptr->phys_addr + +addr_offset, the_area_ptr->rwx);
    }
    else {
        seg_fault();
    }
}

void map_one_page_rwx(unsigned long *pgd_p, unsigned long va, unsigned long pa, unsigned long rwxflag) {
    unsigned long flag = 0;
    // read
    if (rwxflag & 0b1)
        flag |= PD_UK_ACCESS;
    // write
    if (!(rwxflag & 0b10))
        flag |= PD_RDONLY;
    // execute
    if (!(rwxflag & 0b100))
        flag |= PD_UNX;
    // map page
    map_one_page(pgd_p, va, pa, flag);
}

void seg_fault() {
    uart_printf("[Segmentation fault]: Kill Process\r\n");
    thread_exit();
}
