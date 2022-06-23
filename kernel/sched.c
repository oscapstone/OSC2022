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

    thread_t* idlethread = thread_create(idle);
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
    switch_to(get_current(), &curr_thread->context);
    unlock();
}

void kill_zombies() {
    lock();
    list_head_t *curr;
    list_for_each(curr, run_queue) {
        if (((thread_t *)curr)->iszombie) {
            list_del_entry(curr);
            ((thread_t *)curr)->isused = 0;
            ((thread_t *)curr)->iszombie = 0;
            // free user stack & kernel stack
            kfree(((thread_t *)curr)->stack_alloced_ptr);
            kfree(((thread_t *)curr)->kernel_stack_alloced_ptr);
        }
    }
    unlock();
}

thread_t *thread_create(void *start) {
    lock();
    thread_t *r;
    for (int i = 0; i <= PIDMAX; i++) {
        if (!threads[i].isused) {
            r = &threads[i];
            break;
        }
    }

    r->isused = 1;
    r->iszombie = 0;
    r->context.lr = (unsigned long long)start;
    r->stack_alloced_ptr = kmalloc(USTACK_SIZE);
    r->kernel_stack_alloced_ptr = kmalloc(KSTACK_SIZE);
    r->context.sp = (unsigned long long )r->stack_alloced_ptr + USTACK_SIZE;
    r->context.fp = r->context.sp;
    r->signal_is_checking = 0;
    memset(r->curr_working_dir, 0, 256);
    memcpy(r->curr_working_dir, "/", 1);

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
    thread_t *t = thread_create(data);
    t->data = kmalloc(filesize);
    t->datasize = filesize;
    t->context.lr = (unsigned long)t->data;
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
        "eret\n\t"
        :: "r"(&t->context), "r"(t->context.lr), "r"(t->context.sp), "r"(t->kernel_stack_alloced_ptr + KSTACK_SIZE)
    );
    return 0;
}

void schedule_timer(char *notuse) {
    unsigned long long cntfrq_el0;
    asm volatile("mrs %0, cntfrq_el0\n\t" : "=r"(cntfrq_el0));
    add_timer(schedule_timer, cntfrq_el0 >> 5, "", 1);
}
