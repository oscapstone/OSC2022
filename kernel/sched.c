#include "sched.h"

thread_t *curr_thread;
list_head_t *run_queue;
list_head_t *wait_queue;
thread_t threads[PIDMAX + 1];

void init_thread_sched() {
    lock();
    run_queue = kmalloc(sizeof(list_head_t));
    wait_queue = kmalloc(sizeof(list_head_t));
    INIT_LIST_HEAD(run_queue);
    INIT_LIST_HEAD(wait_queue);

    // init pids
    for (int i = 0; i <= PIDMAX; i++) {
        threads[i].pid = i;
        threads[i].isused = 0;
        threads[i].iszombie = 0;
    }
    // malloc a space for current kernel thread to prevent crash
    asm volatile("msr tpidr_el1, %0" :: "r"(kmalloc(sizeof(thread_t))));

    thread_t* idlethread = thread_create(idle, 0x1000);
    curr_thread = idlethread;
    unlock();
}

void idle() {
    while (1) {
        kill_zombies();
        schedule();
    }
}

void schedule() {
    lock();
    do {
        curr_thread = (thread_t *)curr_thread->listhead.next;
    } while (list_is_head(&curr_thread->listhead, run_queue) || curr_thread->iszombie);
    unlock();
    switch_to(get_current(), &curr_thread->context);
}

void kill_zombies() {
    lock();
    list_head_t *curr;
    list_for_each(curr, run_queue) {
        if (((thread_t *)curr)->iszombie) {
            list_del_entry(curr);
            kfree(((thread_t *)curr)->kernel_stack_alloced_ptr);
            // free tables (VA)
            free_page_tables(((thread_t *)curr)->context.ttbr0_el1, 0);
            // free alloced area and vma struct
            list_head_t *pos = ((thread_t *)curr)->vma_list.next;
            while (pos != &((thread_t *)curr)->vma_list) {
                if (((vm_area_struct_t *)pos)->is_alloced)
                    kfree((void*)PHYS_TO_VIRT(((vm_area_struct_t *)pos)->phys_addr));
                list_head_t* next_pos = pos->next;
                kfree(pos);
                pos = next_pos;
            }
            // free PGD
            kfree(PHYS_TO_VIRT(((thread_t *)curr)->context.ttbr0_el1));
            ((thread_t *)curr)->isused = 0;
            ((thread_t *)curr)->iszombie = 0;
            // free user stack & kernel stack
            // kfree(((thread_t *)curr)->stack_alloced_ptr);
        }
    }
    unlock();
}

thread_t *thread_create(void *start, unsigned int filesize) {
    lock();
    thread_t *r;
    for (int i = 0; i <= PIDMAX; i++) {
        if (!threads[i].isused) {
            r = &threads[i];
            break;
        }
    }

    INIT_LIST_HEAD(&r->vma_list);
    r->isused = 1;
    r->iszombie = 0;
    r->signal_is_checking = 0;
    r->context.lr = (unsigned long long)start;
    r->stack_alloced_ptr = kmalloc(USTACK_SIZE);
    r->kernel_stack_alloced_ptr = kmalloc(KSTACK_SIZE);
    r->data = kmalloc(filesize);
    r->datasize = filesize;
    r->context.sp = (unsigned long long )r->kernel_stack_alloced_ptr + KSTACK_SIZE;
    r->context.fp = r->context.sp;
    
    r->context.ttbr0_el1 = kmalloc(0x1000);
    memset(r->context.ttbr0_el1, 0, 0x1000);
    // initial signal handler with signal_default_handler (kill thread)
    for (int i = 0; i < SIGNAL_MAX; i++) {
        r->singal_handler[i] = signal_default_handler;
        r->sigcount[i] = 0;
    }

    list_add(&r->listhead, run_queue);
    unlock();
    return r;
}

void thread_exit() {
    lock();
    curr_thread->iszombie = 1;
    unlock();
    schedule();
}

int exec_thread(char *data, unsigned int filesize) {
    thread_t *t = thread_create(data, filesize);
    // device memory
    add_vma(t, 0x3C000000L, 0x3000000L, 0x3C000000L, 3, 0);
    // user stack
    add_vma(t, 0xffffffffb000, 0x4000, (unsigned long)VIRT_TO_PHYS(t->stack_alloced_ptr), 7, 1);
    // code
    add_vma(t, 0x0, filesize, (unsigned long)VIRT_TO_PHYS(t->data), 7, 1);
    // signal wrapper
    add_vma(t, USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED, 0x2000, (unsigned long)VIRT_TO_PHYS(signal_handler_wrapper), 5, 0);

    t->context.ttbr0_el1 = VIRT_TO_PHYS(t->context.ttbr0_el1);
    t->context.sp = 0xfffffffff000;
    t->context.fp = 0xfffffffff000;
    t->context.lr = 0L;
    // copy file into data
    for (int i = 0; i < filesize; i++)
        t->data[i] = data[i];
    
    curr_thread = t;
    add_timer(schedule_timer, 1, "", 0);
    // eret to EL0
    asm volatile(
        "msr spsr_el1, xzr\n\t"
        "msr tpidr_el1, %0\n\t"
        "msr elr_el1, %1\n\t"
        "msr sp_el0, %2\n\t"
        "mov sp, %3\n\t"
        "dsb ish\n\t"        // ensure write has completed
        "msr ttbr0_el1, %4\n\t"
        "tlbi vmalle1is\n\t" // invalidate all TLB entries
        "dsb ish\n\t"        // ensure completion of TLB invalidatation
        "isb\n\t"            // clear pipeline"
        "eret\n\t"
        :: "r"(&t->context), "r"(t->context.lr), "r"(t->context.sp), "r"(t->kernel_stack_alloced_ptr + KSTACK_SIZE), "r"(t->context.ttbr0_el1)
    );
    return 0;
}

void schedule_timer(char *notuse) {
    unsigned long long cntfrq_el0;
    asm volatile("mrs %0, cntfrq_el0\n\t" : "=r"(cntfrq_el0));
    add_timer(schedule_timer, cntfrq_el0 >> 5, "", 1);
}
