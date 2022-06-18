#ifndef _MMU_H_
#define _MMU_H_
#include "mm/pgtable.h"
#include "types.h"
#include "asm.h"

typedef uint64_t pgdval_t;
typedef uint64_t pudval_t;
typedef uint64_t pmdval_t;
typedef uint64_t pteval_t;

#define UPPER_ADDR_SPACE_BASE ((uint64_t)0xFFFF000000000000)
#define PAGE_SHIFT 12
#define PAGE_SIZE (1ul << PAGE_SHIFT)


#define pgd_index(addr) (((uint64_t)addr >> PGD_SHIFT) & 0b111111111)
#define pgd_offset(pgd, addr) ((pgdval_t*)pgd + pgd_index(addr))
#define pgd_none(val) (val == 0)
#define pgd_set(pgd, val) do{*((pgdval_t*)pgd) = val;}while(0)

#define pud_index(addr) (((uint64_t)addr >> PUD_SHIFT) & 0b111111111)
#define pud_offset(pgd_e, addr) ((pudval_t*)phys_to_virt(((uint64_t)*pgd_e & PHYS_ADDR_MASK)) + pud_index(addr))
#define pud_none(val) (val == 0)
#define pud_set(pud, val) do{*((pudval_t*)pud) = val;}while(0)

#define pmd_index(addr) (((uint64_t)addr >> PMD_SHIFT) & 0b111111111)
#define pmd_offset(pud_e, addr) ((pmdval_t*)phys_to_virt(((uint64_t)*pud_e & PHYS_ADDR_MASK)) + pmd_index(addr))
#define pmd_none(val) (val == 0)
#define pmd_set(pmd, val) do{*((pmdval_t*)pmd) = val;}while(0)

#define pte_index(addr) (((uint64_t)addr >> PTE_SHIFT) & 0b111111111)
#define pte_offset(pmd_e, addr) ((pteval_t*)phys_to_virt(((uint64_t)*pmd_e & PHYS_ADDR_MASK)) + pte_index(addr))
#define pte_none(val) (val == 0)
#define pte_set(pte, val) do{*((pteval_t*)pte) = val;}while(0)
#define pte_page(pte_e) ((struct page*)phys_to_page(((uint64_t)*pte_e & PHYS_ADDR_MASK)))
#define pte_reuse(pte_e) do{ \
                            uint64_t daif = local_irq_disable_save(); \
                            struct page* page = phys_to_page(((uint64_t)*pte_e & PHYS_ADDR_MASK)); \
                            page->ref_cnt++; \
                            local_irq_restore(daif); \
                        }while(0)

#define pte_inuse(pte_e) (((struct page*)phys_to_page(((uint64_t)*pte_e & PHYS_ADDR_MASK)))->ref_cnt != 0)
#define pte_ref_cnt(pte_e) (((struct page*)phys_to_page(((uint64_t)*pte_e & PHYS_ADDR_MASK)))->ref_cnt)

#define VM_FAULT_NONE 0 
#define VM_FAULT_BADMAP 1 
#define VM_FAULT_BADACCESS 2 

#define VM_USER_SPACE 0xffffffffffff 

extern void page_init();
extern void mappages(pgdval_t*, uint64_t, uint64_t, uint64_t, uint64_t);
extern void dup_pages(pgdval_t*, pgdval_t*, uint64_t, uint64_t, uint64_t);
extern void free_page_table(pgdval_t*);
extern void free_one_pgd(pgdval_t*);
extern void free_one_pud(pudval_t*);
extern void free_one_pmd(pmdval_t*);
extern void free_one_pte(pteval_t*);
extern void do_mem_abort(uint64_t, uint64_t);
extern pteval_t* get_pte(pgdval_t*, uint64_t);
#endif
