#include <mmu.h>
#include <kmalloc.h>
#include <initrd.h>
#include <sched.h>
#include <process.h>
#include <error.h>
#include <string.h>
#include <signal.h>

uint64_t *mmu_get_PTE_entry(uint64_t ttbr0, uint64_t va, uint8_t create)
{
    ttbr0 = kernel_pa_to_va(ttbr0);
    uint64_t *PGD = (uint64_t*)ttbr0;
    uint64_t PUD_idx = (va >> 39) & 0b111111111;
    if((PGD[PUD_idx]&1) == 0){
        if(!create) return 0;
        uint64_t new_PUD = (uint64_t)buddy_calloc(1);
        PGD[PUD_idx] = (uint64_t)kernel_va_to_pa(new_PUD | PD_TABLE);
    }
    uint64_t *PUD = (uint64_t*)kernel_pa_to_va((PGD[PUD_idx]>>12)<<12);
    uint64_t PMD_idx = (va >> 30) & 0b111111111;
    if((PUD[PMD_idx]&1) == 0){
        if(!create) return 0;
        uint64_t new_PMD = (uint64_t)buddy_calloc(1);
        PUD[PMD_idx] = (uint64_t)kernel_va_to_pa(new_PMD | PD_TABLE);
    }
    uint64_t *PMD = (uint64_t*)kernel_pa_to_va((PUD[PMD_idx]>>12)<<12);
    uint64_t PTE_idx = (va >> 21) & 0b111111111;
    if((PMD[PTE_idx]&1) == 0){
        if(!create) return 0;
        uint64_t new_PTE = (uint64_t)buddy_calloc(1);
        PMD[PTE_idx] = (uint64_t)kernel_va_to_pa(new_PTE | PD_TABLE);
    }
    uint64_t *PTE = (uint64_t*)kernel_pa_to_va((PMD[PTE_idx]>>12)<<12);
    uint64_t PE_idx = (va >> 12) & 0b111111111;
    return &PTE[PE_idx];
}

int mmu_map_anonymous(uint64_t ttbr0, uint64_t va, uint64_t pages, int prot)
{
    //va = (va >> PAGE_SIZE) << PAGE_SIZE;
    if(va & 0xfff) return -1;
    uint64_t *PTE_entry = 0;
    for(uint64_t i=0;i<pages;i++,PTE_entry++){
        if(((uint64_t)PTE_entry & 0b111111111) == 0){
            PTE_entry = mmu_get_PTE_entry(ttbr0, va + (i<<PAGE_SIZE), 1);
            //kmsg("PTE Entry: 0x%x", PTE_entry);
        }
        if(!PTE_entry){
            kpanic("Couldn't get PTE entry.");
        }
        if((*PTE_entry & 0b11) == 0b11) return -1;
    }
    PTE_entry = 0;
    uint64_t page_attr = PROC_PTE_ATTR_NORMAL;
    if((prot & PROT_READ)) page_attr |= PD_USERACCESS;
    if(!(prot & PROT_WRITE)) page_attr |= PD_RO;
    if(!(prot & PROT_EXEC)) page_attr |= PD_UNX;
    for(uint64_t i=0;i<pages;i++,PTE_entry++){
        if(((uint64_t)PTE_entry & 0b111111111) == 0){
            PTE_entry = mmu_get_PTE_entry(ttbr0, va + (i<<PAGE_SIZE), 1);
            kmsg("PTE Entry: 0x%x", PTE_entry);
        }
        if(!PTE_entry){
            kpanic("Couldn't get PTE entry.");
        }
        uint64_t alloc_addr = buddy_calloc(1);
        if(alloc_addr==0) return -1;
        *PTE_entry = kernel_va_to_pa(((uint64_t)alloc_addr)) | page_attr;
    }
    return 0;
}

void mmu_page_fault_handler(uint64_t va)
{
    kmsg("handle 0x%x", va);
    if(va & 0xffff000000000000){
        kpanic("Kernel Page Fault: 0x%x", va);
    }

    Thread *thread = get_thread(thread_get_current());
    Process *process = thread->process;
    if(!process){
        kpanic("Pagefault at userspace, but doesn't has process running. (Pagefault: 0x%x)", va);
        //schedule();
    }

    MemoryRegion *cur_mr = process->process_memory_region;
    while(cur_mr){
        if(va >= cur_mr->VA_base && va < cur_mr->VA_base + (cur_mr->pages << PAGE_SIZE)){
            uint64_t *va_aligned = (va >> PAGE_SIZE) << PAGE_SIZE;
            if(mmu_map_anonymous(process->ttbr0_el0, (uint64_t)va_aligned, 1, cur_mr->prot) != 0){
                goto process_pagefault;
            }
            if(!(cur_mr->flags & MAP_ANONYMOUS)){
                INITRD_FILE *fp = initrd_get(cur_mr->filepath);
                uint64_t offset = (uint64_t)va_aligned - cur_mr->VA_base;
                uint64_t copy_len = 1 << PAGE_SIZE;
                if(offset < fp->filesize){
                    if(fp->filesize - offset < copy_len) copy_len = fp->filesize - offset;
                    memcpy(va_aligned, fp->filecontent+offset, copy_len);
                }
            }
            return ;
        }
        cur_mr = cur_mr->next;
    }

    process_pagefault:
    kmsg("Process PageFault!!");
    signal_kill(thread->process->pid, 11);
    return ;
}

int mmu_copy_process_mem(Process* dst, Process* src)
{
    dst->ttbr0_el0 = kernel_va_to_pa((uint64_t)buddy_calloc(1));
    MemoryRegion* src_cur_mr = src->process_memory_region;
    MemoryRegion* dst_cur_mr = 0;
    while(src_cur_mr){
        if(!dst_cur_mr){
            dst_cur_mr = (MemoryRegion *)kmalloc(sizeof(MemoryRegion));
            dst->process_memory_region = dst_cur_mr;
        }
        else{
            dst_cur_mr->next = (MemoryRegion *)kmalloc(sizeof(MemoryRegion));
            dst_cur_mr = dst_cur_mr->next;
        }
        dst_cur_mr->next = 0;
        dst_cur_mr->filepath = src_cur_mr->filepath;
        dst_cur_mr->prot = src_cur_mr->prot;
        dst_cur_mr->flags = src_cur_mr->flags;
        dst_cur_mr->pages = src_cur_mr->pages;
        dst_cur_mr->VA_base = src_cur_mr->VA_base;
        kmsg("Copy pagetable to dst process: VA=0x%x, pages=0x%x",dst_cur_mr->VA_base, dst_cur_mr->pages);

        for(int i=0;i<src_cur_mr->pages;i++){
            uint64_t va = src_cur_mr->VA_base + (i << PAGE_SIZE);
            uint64_t *src_PTE_entry = mmu_get_PTE_entry(src->ttbr0_el0, va, 0);
            if(!src_PTE_entry) continue;
            uint64_t *dst_PTE_entry = mmu_get_PTE_entry(dst->ttbr0_el0, va, 1);
            if(!dst_PTE_entry) continue;
            kmsg("Copy PTE 0x%x: 0x%x", va, *src_PTE_entry);
            *dst_PTE_entry = *src_PTE_entry;
            *dst_PTE_entry |= PD_RO;
            *src_PTE_entry |= PD_RO;
            if((*src_PTE_entry) & PD_ACCESS){
                uint64_t *mem = (uint64_t*)kernel_pa_to_va(((*src_PTE_entry)>>12)<<12);
                buddy_ref(mem);
            }
        }

        src_cur_mr = src_cur_mr->next;
    }
    return 0;
}

void mmu_page_permission_handler(uint64_t va, uint64_t esr_el1)
{
    if(va & 0xffff000000000000){
        kpanic("Kernel page permission error: 0x%x", va);
    }

    Thread *thread = get_thread(thread_get_current());
    Process *process = thread->process;
    if(!process){
        kpanic("Page permission fault at userspace, but doesn't has process running. (Pagefault: 0x%x)", va);
        //schedule();
    }
    
    if(esr_el1 >> 26 == 0b100100 && ((esr_el1 >> 6) & 1)){ // Data Write, Maybe Copy On Write
        MemoryRegion *cur_mr = process->process_memory_region;
        while(cur_mr){
            if(va >= cur_mr->VA_base && va < cur_mr->VA_base + (cur_mr->pages << PAGE_SIZE)){
                uint64_t *va_aligned = (va >> PAGE_SIZE) << PAGE_SIZE;
                if(!(cur_mr->prot & PROT_WRITE)) goto segfault;
                //CoW
                uint64_t* PTE_entry = mmu_get_PTE_entry(process->ttbr0_el0, va_aligned, 0);
                if(!PTE_entry) goto segfault;
                char* mem = (char*)kernel_pa_to_va(((*PTE_entry)>>12)<<12);
                if(!mem) goto segfault;
                char* newmem = (char *)buddy_alloc(1);
                memcpy(newmem, mem, 1<<PAGE_SIZE);
                /*
                kmsg("diff   newmem  |  old");
                for(int i=0;i<(1<<(PAGE_SIZE-3));i++){
                    kmsg("0x%x | 0x%x", *(uint64_t*)(newmem+(i*8)), *(uint64_t*)(mem+(i*8)));
                }
                */
                uint64_t newPTE = ((uint64_t)kernel_va_to_pa((uint64_t)newmem) & (~0xfff)) | ((*PTE_entry) & 0xfff);
                newPTE &= ~(uint64_t)PD_RO;
                *PTE_entry = newPTE;
                kmsg("CoW: pid=%d va=0x%x oldmem=0x%x newmem=0x%x", process->pid, va, mem, newmem);
                asm("tlbi vmalle1is");
                asm("dsb ish");
                asm("isb");
                buddy_free(mem);
                return ;
            }
            cur_mr = cur_mr->next;
        }
    }
    
    segfault:
    kmsg("Page permission error: 0x%x", va);
    signal_kill(thread->process->pid, 11);
    return ;
}

int mmu_map_peripheral(uint64_t ttbr0)
{
    for(uint64_t va = 0x30000000; va < 0x40010000; va += (1<<PAGE_SIZE)){
        uint64_t *PTE_entry = mmu_get_PTE_entry(ttbr0, va, 1);
        uint64_t newPTE = va | PROC_PTE_ATTR_DEVICE | PD_USERACCESS;
        *PTE_entry = newPTE;
    }
}

int mmu_new_mr(MemoryRegion **mr, MemoryRegion **retmr, uint64_t va, size_t len, int prot, int flags, char *filepath)
{
    prot &= 0b111;
    flags &= MAP_ANONYMOUS | MAP_POPULATE;
    va = (va >> PAGE_SIZE) << PAGE_SIZE;
    len = (((len-1) >> PAGE_SIZE) + 1)<<PAGE_SIZE;

    int found = 0;
    MemoryRegion *new_mr = (MemoryRegion *)kmalloc(sizeof(MemoryRegion));
    new_mr->VA_base = 0x0;
    new_mr->filepath = 0x0;
    new_mr->flags = flags;
    new_mr->prot = prot;
    new_mr->next = 0;
    new_mr->pages = len >> PAGE_SIZE;
    if(!(flags & MAP_ANONYMOUS)) new_mr->filepath = filepath;
    MemoryRegion *cur_mr = *mr;
    uint64_t cur_va_l = 0x0;
    uint64_t cur_va_r = cur_mr ? (cur_mr->VA_base) : 0x1000000000000;
    uint64_t va_r_bound = 0x0;
    uint64_t diff_with_req = 0xffffffffffffffff;
    MemoryRegion *to_insert = 0;
    if(cur_va_r-cur_va_l >= len){
        new_mr->VA_base = cur_va_l;
        va_r_bound = cur_va_r;
        diff_with_req = (va >= new_mr->VA_base) ? (va - new_mr->VA_base) : (new_mr->VA_base - va);
        found = 1;
    }
    while(cur_mr){
        cur_va_l = cur_mr->VA_base + (cur_mr->pages << PAGE_SIZE);
        cur_va_r = cur_mr->next ? cur_mr->next->VA_base : 0x1000000000000;
        if(cur_va_r-cur_va_l >= len){
            uint64_t new_diff = (va >= cur_va_l) ? (va - cur_va_l) : (cur_va_l - va);
            if(new_diff < diff_with_req){
                new_mr->VA_base = cur_va_l;
                va_r_bound = cur_va_r;
                diff_with_req = new_diff;
                found = 1;
                to_insert = cur_mr;
            }
        }
        if(cur_va_l >= va && found) break;
        cur_mr = cur_mr->next;
    }
    if(!found){
        kmsg("Out of virtual address space.");
        return -1;
    }
    va_r_bound = (va_r_bound >> PAGE_SIZE) << PAGE_SIZE;
    if(new_mr->VA_base < va && va_r_bound){
        new_mr->VA_base = va_r_bound - len;
    }
    if(to_insert){
        new_mr->next = to_insert->next;
        to_insert->next = new_mr;
    }
    else{
        new_mr->next = *mr;
        *mr = new_mr;
    }
    *retmr = new_mr;

    return 0;
}

void* mmap(void* addr, size_t len, int prot, int flags)
{
    kmsg("mmap(addr=0x%x, len=%d, prot=0x%x, flags=0x%x)", addr, len, prot, flags);
    Thread *thread = get_thread(thread_get_current());
    if(!thread){
        kmsg("Couldn't get current thread.");
        return 0;
    }
    Process *process = thread->process;
    if(!process){
        kmsg("Couldn't found process in thread (thread: %d)", thread->thread_id);
        return 0;
    }

    MemoryRegion *found_mr = 0;
    mmu_new_mr(&(process->process_memory_region), &found_mr, addr, len, prot, flags, 0);
    if(!found_mr){
        kmsg("mmap unsuccessful. addr=0x%x, len=%d, prot=0x%x, flags=0x%x", addr, len, prot, flags);
        return 0;
    }
    kmsg("mmap success: addr=0x%x pages=%d", found_mr->VA_base, found_mr->pages);
    return found_mr->VA_base;
}