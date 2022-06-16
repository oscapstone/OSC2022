#include "mm/mmu.h"
extern int __reserved_page_table_start, __reserved_page_table_end;
extern int __PGD_start, __PGD_end;
extern int __PUD_start, __PUD_end;
extern int __PMD_start, __PMD_end;
extern int __PTE_start, __PTE_end;
extern struct mem_node memory_node;

void page_init(){
    uint64_t daif = local_irq_disable_save();
    uint64_t pgt_start = (uint64_t)&__reserved_page_table_start;
    uint64_t pgt_end = (uint64_t)&__reserved_page_table_end;
    uint64_t pgd = (uint64_t)&__PGD_start;
    uint64_t pud = (uint64_t)&__PUD_start;
    uint64_t pmd = (uint64_t)&__PMD_start;
    uint64_t pte = (uint64_t)&__PTE_start;
    pgdval_t* tmp_pgd = (pgdval_t*)pgd;
    pudval_t* tmp_pud = (pudval_t*)pud;
    pmdval_t* tmp_pmd = (pmdval_t*)pmd;
    pteval_t* tmp_pte = (pteval_t*)pte;
    uint64_t mem_size = ALIGN_UP(memory_node.end - memory_node.start, PAGE_SIZE);
    uint64_t max_page_num = mem_size / PAGE_SIZE;

    pgd = virt_to_phys(pgd);
    pud = virt_to_phys(pud);
    pmd = virt_to_phys(pmd);
    pte = virt_to_phys(pte);

   // memset((void*)pgt_start, 0, pgt_end - pgt_start);

    INFO("mem_size: %p", mem_size);
    INFO("max number of page: %p", max_page_num);

    INFO("Initialize 4-level page table for upper address space");

    INFO("level 4 page table at %p...", tmp_pte);
    for(uint64_t i = 0 ; i < max_page_num ; i++){
        pte_set(tmp_pte, VM_PTE_NORMAL_ATTR | (i * PAGE_SIZE));
        tmp_pte++;
    }

    for(uint64_t i = max_page_num ; i < 2 * 512 * 512; i++){
        pte_set(tmp_pte, VM_PTE_DEVICE_ATTR | (i * PAGE_SIZE));
        tmp_pte++;
    }

    INFO("level 3 page table %p...", tmp_pmd);
    for(uint64_t i = 0 ; i < 2 * 512 ; i++){
        pmd_set(tmp_pmd, VM_PMD_ATTR | (pte + i * PAGE_SIZE));
        tmp_pmd++;
    }
    
    INFO("level 2 page table %p...", tmp_pud);
    for(uint64_t i = 0 ; i < 2 ; i++){
        pud_set(tmp_pud, VM_PUD_ATTR | (pmd + i * PAGE_SIZE));
        tmp_pud++;
    }

    INFO("level 1 page table %p...", tmp_pgd);
    asm volatile(
        "dsb ish\n\t"           // ensure write has completed
        "tlbi vmalle1is\n\t"    // invalidate all TLB entries 
        "dsb ish\n\t"           // ensure completion of TLB invalidatation 
        "isb\n\t"               // clear pipeline 
    );
    local_irq_restore(daif);
    INFO("finish");
}

void mappages(pgdval_t* pgd, uint64_t va, uint64_t pa, uint64_t size, uint64_t prot){
    uint64_t vstart, vend;
    uint64_t pgtable = 0;
    pudval_t* pud;
    pmdval_t* pmd;
    pteval_t* pte;
    size = ALIGN_UP(size, PAGE_SIZE);
    for(vstart = va, vend = va + size ; vstart != vend ; vstart += PAGE_SIZE){
        pud = pud_offset(pgd, vstart);
        if(pud_none(*pud)){
            pgtable = virt_to_phys(calloc_page());
            pud_set(pud, PUD_TYPE_TABLE | pgtable);
        }
    }
}

