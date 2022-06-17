#include "mm/mmu.h"
#include "mm/mm.h"
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
    pgdval_t* tmp_pgd_e = (pgdval_t*)pgd;
    pudval_t* tmp_pud_e = (pudval_t*)pud;
    pmdval_t* tmp_pmd_e = (pmdval_t*)pmd;
    pteval_t* tmp_pte_e = (pteval_t*)pte;
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

    INFO("level 4 page table at %p...", tmp_pte_e);
    for(uint64_t i = 0 ; i < max_page_num ; i++){
        pte_set(tmp_pte_e, VM_PTE_NORMAL_ATTR | (i * PAGE_SIZE));
        tmp_pte_e++;
    }

    for(uint64_t i = max_page_num ; i < 2 * 512 * 512; i++){
        pte_set(tmp_pte_e, VM_PTE_DEVICE_ATTR | (i * PAGE_SIZE));
        tmp_pte_e++;
    }

    INFO("level 3 page table %p...", tmp_pmd_e);
    for(uint64_t i = 0 ; i < 2 * 512 ; i++){
        pmd_set(tmp_pmd_e, VM_PMD_ATTR | (pte + i * PAGE_SIZE));
        tmp_pmd_e++;
    }
    
    INFO("level 2 page table %p...", tmp_pud_e);
    for(uint64_t i = 0 ; i < 2 ; i++){
        pud_set(tmp_pud_e, VM_PUD_ATTR | (pmd + i * PAGE_SIZE));
        tmp_pud_e++;
    }

    INFO("level 1 page table %p...", tmp_pgd_e);
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
    LOG("mappages start");
    uint64_t vstart, vend;
    uint64_t pgtable = 0;
    pudval_t* pgd_e;
    pudval_t* pud_e;
    pmdval_t* pmd_e;
    pteval_t* pte_e;
    size = ALIGN_UP(size, PAGE_SIZE);
    LOG("mappages(%p, %p, %p, %l, %p)", pgd, va, pa, size, prot);
    for(vstart = va, vend = va + size ; vstart != vend ; vstart += PAGE_SIZE){
        LOG("physical addr: %p", pa);
        pgd_e = pgd_offset(pgd, vstart);
        if(pgd_none(*pgd_e)){
            pgtable = virt_to_phys(calloc_page());
            pgd_set(pgd_e, PGD_TYPE_TABLE | pgtable);
        }
        LOG("pgd_e: %p, *pgd_e = %p\r\n",pgd_e, *pgd_e);

        pud_e = pud_offset(pgd_e, vstart);
        if(pud_none(*pud_e)){
            pgtable = virt_to_phys(calloc_page());
            pud_set(pud_e, PUD_TYPE_TABLE | pgtable);
        }
        LOG("pud_e: %p, *pud_e = %p\r\n",pud_e, *pud_e);

        pmd_e = pmd_offset(pud_e, vstart);
        if(pmd_none(*pmd_e)){
            pgtable = virt_to_phys(calloc_page());
            pmd_set(pmd_e, PMD_TYPE_TABLE | pgtable);
        }
        LOG("pmd_e: %p, *pmd_e = %p\r\n",pmd_e, *pmd_e);

        pte_e = pte_offset(pmd_e, vstart);
        //printf("pte_index(%p) = %p\r\n", vstart, pte_index(vstart));
        if(!pte_none(*pte_e)){
            printf("Try to modify the physical address of a pte entry to another address\r\n");
            while(1);
        }
        pte_set(pte_e, prot | PAGE_ATTR_AF | (pa & PHYS_ADDR_MASK));
        LOG("pte_e: %p, *pte_e = %p\r\n",pte_e, *pte_e);
        pa += PAGE_SIZE;
    }
    LOG("mappages end");
}

void dup_pages(pgdval_t* dst_pgd, pgdval_t* src_pgd, uint64_t va, uint64_t size, uint64_t prot){
    LOG("dup_pages start");
    uint64_t vstart, vend;
    uint64_t pgtable = 0;
    pudval_t* tmp_pgd_e, *pgd_e;
    pudval_t* tmp_pud_e, *pud_e;
    pmdval_t* tmp_pmd_e, *pmd_e;
    pteval_t* tmp_pte_e, *pte_e;
    size = ALIGN_UP(size, PAGE_SIZE);
    LOG("dup_pages(%p, %p, %p, %p, %p)", dst_pgd, src_pgd, va, size, prot);
    for(vstart = va, vend = va + size ; vstart != vend ; vstart += PAGE_SIZE){
        tmp_pgd_e = pgd_offset(src_pgd, vstart);
        pgd_e = pgd_offset(dst_pgd, vstart);
        if(pgd_none(*pgd_e)){
            pgtable = virt_to_phys(calloc_page());
            pgd_set(pgd_e, (*tmp_pgd_e & ~PHYS_ADDR_MASK) | pgtable);
        }
        LOG("tmp_pgd_e: %p, *tmp_pgd_e = %p",tmp_pgd_e, *tmp_pgd_e);
        LOG("pgd_e: %p, *pgd_e: %p",pgd_e, *pgd_e);
        

        tmp_pud_e = pud_offset(tmp_pgd_e, vstart);
        pud_e = pud_offset(pgd_e, vstart);
        if(pud_none(*pud_e)){
            pgtable = virt_to_phys(calloc_page());
            pud_set(pud_e, (*tmp_pud_e & ~PHYS_ADDR_MASK) | pgtable);
        }
        LOG("tmp_pud_e: %p, *tmp_pud_e = %p",tmp_pud_e, *tmp_pud_e);
        LOG("pud_e: %p, *pud_e: %p",pud_e, *pud_e);

        tmp_pmd_e = pmd_offset(tmp_pud_e, vstart);
        pmd_e = pmd_offset(pud_e, vstart);
        if(pmd_none(*pmd_e)){
            pgtable = virt_to_phys(calloc_page());
            pmd_set(pmd_e, (*tmp_pmd_e & ~PHYS_ADDR_MASK) | pgtable);
        }
        LOG("tmp_pmd_e: %p, *tmp_pmd_e = %p",tmp_pmd_e, *tmp_pmd_e);
        LOG("pmd_e: %p, *pmd_e: %p",pmd_e, *pmd_e);

        tmp_pte_e = pte_offset(tmp_pmd_e, vstart);
        pte_e = pte_offset(pmd_e, vstart);
        pte_set(pte_e, *tmp_pte_e | prot);
        // add 1 to page reference count 
        pte_reuse(pte_e);
        LOG("tmp_pte_e: %p, *tmp_pte_e = %p",tmp_pte_e, *tmp_pte_e);
        LOG("pte_e: %p, *pte_e: %p, pte_ref_cnt(%p) = %p ",pte_e, *pte_e, pte_e, pte_ref_cnt(pte_e));
    }
    LOG("dup_pages end");
}
void free_one_pte(pteval_t* pte){
    LOG("free_one_pte(%p)", pte);
    pteval_t* pte_e = pte;
    void* page_frame;
    for(uint64_t i = 0 ; i < 512 ; i++){
        if(!pte_none(pte_e[i])){
            page_frame = (void*)phys_to_virt(pte_e[i] & PHYS_ADDR_MASK);
            LOG("free page frame: %p", page_frame);
            free_page(page_frame);
        }
    }
    free_page(pte);
}

void free_one_pmd(pmdval_t* pmd){
    LOG("free_one_pmd(%p)", pmd);
    pmdval_t* pmd_e = pmd;
    pteval_t* pte;
    for(uint64_t i = 0 ; i < 512 ; i++){
        if(!pmd_none(pmd_e[i])){
            pte = (pteval_t*)phys_to_virt(pmd_e[i] & PHYS_ADDR_MASK);
            free_one_pte(pte);
        }
    }
    free_page(pmd);
}

void free_one_pud(pudval_t* pud){
    LOG("free_one_pud(%p)", pud);
    pudval_t* pud_e = pud;
    pmdval_t* pmd;
    for(uint64_t i = 0 ; i < 512 ; i++){
        if(!pud_none(pud_e[i])){
            pmd = (pmdval_t*)phys_to_virt(pud_e[i] & PHYS_ADDR_MASK);
            free_one_pmd(pmd);
        }
    }
    free_page(pud);
}

void free_one_pgd(pgdval_t* pgd){
    LOG("free_one_pgd(%p)", pgd);
    pgdval_t* pgd_e = pgd;
    pudval_t* pud;
    for(uint64_t i = 0 ; i < 512 ; i++){
        if(!pgd_none(pgd_e[i])){
            pud = (pudval_t*)phys_to_virt(pgd_e[i] & PHYS_ADDR_MASK);
            free_one_pud(pud);
        }
    }
    free_page(pgd);
}

void free_page_table(pgdval_t* pgd){
    LOG("free_page_table(%p)", pgd);
    free_one_pgd(pgd);
}
