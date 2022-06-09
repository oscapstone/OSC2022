#include "syscall.h"

// syscall no 0
int getpid(trapframe_t *tpf) {
    tpf->x0 = curr_thread->pid;
    return curr_thread->pid;
}

// syscall no 1
unsigned long uartread(trapframe_t *tpf, char buf[], unsigned long size) {
    int i = 0;
    for (int i = 0; i < size; i++)
        uart_read(&buf[i], 1);
    tpf->x0 = i;
    return i;
}

// syscall no 2
unsigned long uartwrite(trapframe_t *tpf, const char buf[], unsigned long size) {
    int i = 0;
    for (int i = 0; i < size; i++)
        uart_write_char(buf[i]);
    tpf->x0 = i;
    return i;
}

// syscall no 3
int exec(trapframe_t *tpf, const char *name, char *const argv[]) {
    // free alloced area and vma struct
    list_head_t *pos = curr_thread->vma_list.next;
    while (pos != &curr_thread->vma_list) {
        if (((vm_area_struct_t *)pos)->is_alloced)
            kfree((void *)PHYS_TO_VIRT(((vm_area_struct_t *)pos)->phys_addr));
        list_head_t *next_pos = pos->next;
        kfree(pos);
        pos = next_pos;
    }

    INIT_LIST_HEAD(&curr_thread->vma_list);
    curr_thread->datasize = get_file_size((char*)name);
    char *new_data = get_file_start((char *)name);
    // re-alloc code & user stack
    curr_thread->data = kmalloc(curr_thread->datasize);
    curr_thread->stack_alloced_ptr = kmalloc(USTACK_SIZE);

    asm volatile("dsb ish\n\t");      // ensure write has completed
    free_page_tables(curr_thread->context.ttbr0_el1, 0);
    memset(PHYS_TO_VIRT(curr_thread->context.ttbr0_el1), 0, 0x1000);
    asm volatile(
        "tlbi vmalle1is\n\t" // invalidate all TLB entries
        "dsb ish\n\t"        // ensure completion of TLB invalidatation
        "isb\n\t"            // clear pipeline
    );

    // code
    add_vma(curr_thread, 0, curr_thread->datasize, (unsigned long)VIRT_TO_PHYS(curr_thread->data), 7, 1);
    // user stack
    add_vma(curr_thread, 0xffffffffb000, 0x4000, (unsigned long)VIRT_TO_PHYS(curr_thread->stack_alloced_ptr), 7, 1);
    // device memory
    add_vma(curr_thread, 0x3C000000L, 0x3000000L, 0x3C000000L, 3, 0);
    // signal wrapper
    add_vma(curr_thread, USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED, 0x2000, (unsigned long)VIRT_TO_PHYS(signal_handler_wrapper), 5, 0);
    // copy file into data
    memcpy(curr_thread->data, new_data, curr_thread->datasize);

    // clear signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++)
        curr_thread->singal_handler[i] = signal_default_handler;

    // set return address, stack pointer and return value
    tpf->elr_el1 = 0;
    tpf->sp_el0 = 0xfffffffff000;
    tpf->x0 = 0;
    return 0;
}

// syscall no 4
int fork(trapframe_t *tpf) {
    lock();
    thread_t *newt = thread_create(curr_thread->data, curr_thread->datasize);

    // copy signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++)
        newt->singal_handler[i] = curr_thread->singal_handler[i];
    
    // copy vma
    list_head_t *pos;
    list_for_each(pos, &curr_thread->vma_list) {
        // ignore device memory and signal wrapper
        if (((vm_area_struct_t *)pos)->virt_addr == USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED || ((vm_area_struct_t *)pos)->virt_addr == 0x3C000000L)
            continue;
        // alloc new area for child & copy content
        char *new_alloc = kmalloc(((vm_area_struct_t *)pos)->area_size);
        add_vma(newt, ((vm_area_struct_t *)pos)->virt_addr, ((vm_area_struct_t *)pos)->area_size, (unsigned long)VIRT_TO_PHYS(new_alloc), ((vm_area_struct_t *)pos)->rwx, 1);
        memcpy(new_alloc, (void*)PHYS_TO_VIRT(((vm_area_struct_t *)pos)->phys_addr), ((vm_area_struct_t *)pos)->area_size);
    }
    // device memory
    add_vma(newt, 0x3C000000L, 0x3000000L, 0x3C000000L, 3, 0);
    // signal wrapper
    add_vma(newt, USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED, 0x2000, (unsigned long)VIRT_TO_PHYS(signal_handler_wrapper), 5, 0);

    int parent_pid = curr_thread->pid;
    // copy kernel stack into new process
    for (int i = 0; i < KSTACK_SIZE; i++)
        newt->kernel_stack_alloced_ptr[i] = curr_thread->kernel_stack_alloced_ptr[i];

    store_context(get_current());
    
    // child
    if (parent_pid != curr_thread->pid) {
        tpf->x0 = 0;
        return 0;
    }
    // parent
    else {
        void *temp_ttbr0_el1 = newt->context.ttbr0_el1;
        newt->context = curr_thread->context;
        newt->context.ttbr0_el1 = VIRT_TO_PHYS(temp_ttbr0_el1);
        newt->context.fp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr;
        newt->context.sp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr;
        unlock();
        tpf->x0 = newt->pid;
        return newt->pid;
    }    
}

// syscall no 5
void exit(trapframe_t *tpf, int status) {
    thread_exit();
}

// syscall no 6
int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox_user) {
    lock();
    unsigned int size_of_mbox = mbox_user[0];
    memcpy((char *)mbox, mbox_user, size_of_mbox);
    mailbox_call(ch);
    memcpy(mbox_user, (char *)mbox, size_of_mbox);
    tpf->x0 = 8;
    unlock();
    return 0;
}

// syscall no 7
void kill(trapframe_t *tpf, int pid) {
    lock();
    // check for invalid pid
    if (pid >= PIDMAX || pid < 0 || !threads[pid].isused) {
        unlock();
        return;
    }

    threads[pid].iszombie = 1;
    unlock();
    schedule();
}

// syscall no 8
void signal_register(int signal, void (*handler)()) {
    // invalid signal
    if (signal > SIGNAL_MAX || signal < 0)
        return;
    curr_thread->singal_handler[signal] = handler;
}

// syscall no 9
void signal_kill(int pid, int signal) {
    // check for invalid pid
    if (pid >= PIDMAX || pid < 0 || !threads[pid].isused)
        return;
    lock();
    threads[pid].sigcount[signal]++;
    unlock();
}

// syscall no 31
void sigreturn(trapframe_t *tpf) {
    load_context(&curr_thread->signal_saved_context);
}

// syscall no 10 (only implement anonymous page mapping)
void *sys_mmap(trapframe_t *tpf, void *addr, unsigned long len, int prot, int flags, int fd, int file_offset) {
    // rounds up
    len = len % 0x1000 ? len + (0x1000 - len % 0x1000) : len;
    addr = (unsigned long)addr % 0x1000 ? addr + (0x1000 - (unsigned long)addr % 0x1000) : addr;

    // check if overlap
    list_head_t *pos;
    vm_area_struct_t *the_area_ptr = 0;
    list_for_each(pos, &curr_thread->vma_list) {
        if ((unsigned long)(addr + len) > ((vm_area_struct_t *)pos)->virt_addr && (unsigned long)addr < ((vm_area_struct_t *)pos)->virt_addr + ((vm_area_struct_t *)pos)->area_size) {
            the_area_ptr = (vm_area_struct_t *)pos;
            break;
        }
    }
    // if overlap => new region's start address at the end of the area
    if (the_area_ptr) {
        tpf->x0 = (unsigned long)sys_mmap(tpf, (void *)(the_area_ptr->virt_addr + the_area_ptr->area_size), len, prot, flags, fd, file_offset);
        return (void *)tpf->x0;
    }
    // else, add vma
    add_vma(curr_thread, (unsigned long)addr, len, VIRT_TO_PHYS((unsigned long)kmalloc(len)), prot, 1);
    tpf->x0 = (unsigned long)addr;
    return (void*)tpf->x0;
}
