#include "thread.h"
#include "interrupt.h"
#include "exception.h"
#include "memory.h"
#include "timer.h"

thread_t threads[PIDMAX + 1];
thread_t *currThread, *idleThread;
list_head_t *run_queue, *wait_queue;

void initThreads() {
    lock_interrupt();
    run_queue = malloc(sizeof(list_head_t));
    wait_queue = malloc(sizeof(list_head_t));
    INIT_LIST_HEAD(run_queue);
    INIT_LIST_HEAD(wait_queue);

    for(int i = 0; i <= PIDMAX; i++) {
        threads[i].pid = i;
        threads[i].state = IDLE;
        threads[i].has_signal = 0;
    }

    currThread = idleThread = createThread(idle, 0x1000);
    asm volatile("mrs %0, ttbr1_el1" : "=r"(idleThread->context.pgd));
    // uart_printf("IDLE PGD: 0x%x\n", idleThread->context.pgd);
    asm volatile("msr tpidr_el1, %0" ::"r" (&idleThread->context));
    unlock_interrupt();
}

thread_t *createThread(void *program, uint64 datasize) {
    lock_interrupt();
    thread_t *thread;
    for(int i = 0; i <= PIDMAX; i++) {
        if(threads[i].state == IDLE) {
            thread = &threads[i];
            break;
        }
    }

    INIT_LIST_HEAD(&thread->used_vm);
    thread->state = USED;
    thread->context.lr = (uint64)program;
    thread->stackPtr = (char*)malloc(THREAD_STACK_SIZE);
    thread->kernel_stackPtr = (char*)malloc(THREAD_STACK_SIZE);
    thread->data = (char*)malloc(datasize);
    thread->datasize = datasize;

    thread->context.sp = (uint64)thread->stackPtr + THREAD_STACK_SIZE;
    thread->context.fp = thread->context.sp;

    thread->has_signal = 0;
    for(int i = 0; i <= SIGMAX; i++) {
        thread->signal_handlers[i] = signal_default_handlder;
    }

    asm volatile("mrs %0, ttbr0_el1" : "=r"(thread->context.pgd));
    list_add(&thread->listHead, run_queue);
    unlock_interrupt();
    return thread;
}

void idle() {
    while(1) {
        kill_zombies();
        schedule();
    }    
}

void kill_zombies() {
    lock_interrupt();
    list_head_t *list;
    list_for_each(list, run_queue) {
        thread_t *thread = (thread_t*)list;
        if(thread->state == DEAD) {
            list_del_entry(list);
            free(thread->data);
            free(thread->stackPtr);
            free(thread->kernel_stackPtr);
            free_vm_list_for_thread(thread);
            free_page_tables_for_thread(thread); // free PGD
            thread->state = IDLE;
        }
    }
    unlock_interrupt();
}

void schedule() {
    lock_interrupt();
    // uart_printf("Start Schedule\n");
    do {
        if(currThread->state == NEW_BORN) currThread->state = USED;
        currThread = (thread_t*)currThread->listHead.next;
        // uart_printf("D");
    } while(list_is_head(&currThread->listHead, run_queue) || currThread->state == DEAD || currThread->state == NEW_BORN);
    // uart_printf("Select thread: %d\n", currThread->pid);
    switch_to(get_current(), &currThread->context);
    // uart_printf("Switch Back thread: %d\n", currThread->pid);
    unlock_interrupt();
    // uart_printf("Schedule Done\n");
    // return to context.lr
}

void execThread(char *program, uint64 program_size) {
    lock_interrupt();
    thread_t *thread = createThread(program, program_size);
    init_page_table(&thread->context.pgd, 0);
    // uart_printf("New Thread PGD address: 0x%x\n", thread->context.pgd);

    thread->context.sp = USER_STACK_BASE + THREAD_STACK_SIZE;
    thread->context.fp = USER_STACK_BASE + THREAD_STACK_SIZE;
    thread->context.lr = USER_KERNEL_BASE;
    for(int i = 0; i < program_size; i++) {
        thread->data[i] = program[i];
    }

    thread->has_signal = 0;
    for(int i = 0; i <= SIGMAX; i++) {
        thread->signal_handlers[i] = signal_default_handlder;
    }
    
    currThread = thread;
    // set_page_tables_for_thread(thread);
    set_vm_list_for_thread(thread);

    schedule_callback("Schedule\n");
    uart_printf("Start exec\n");

    // uart_printf("Before switch\n");
    // uart_printf("USER Code at 0x%x\n", thread->data);
    // parser_table((uint64)thread->data);
    // uart_printf("USER Stack at 0x%x\n", thread->kernel_stackPtr);
    // parser_table((uint64)thread->kernel_stackPtr);

    switch_pgd((uint64)thread->context.pgd);
    
    // uart_printf("After switch\n");
    // uart_printf("USER Code at 0x%x\n", USER_KERNEL_BASE);
    // parser_table(USER_KERNEL_BASE);
    // uart_printf("USER Stack at 0x%x\n", USER_STACK_BASE);
    // parser_table(USER_STACK_BASE);

    unlock_interrupt();

    // // eret to exception level 0
    asm("msr tpidr_el1, %0\n\t"
        "msr elr_el1, %1\n\t"
        "msr spsr_el1, xzr\n\t" // enable interrupt in EL0. You can do it by setting spsr_el1 to 0 before returning to EL0.
        "msr sp_el0, %2\n\t"
        "mov sp, %3\n\t"
        "eret\n\t" ::"r"(&thread->context), "r"(thread->context.lr), 
                     "r"(thread->context.sp), "r"(thread->kernel_stackPtr + THREAD_STACK_SIZE));
}

void thread_exit() {
    lock_interrupt();
    currThread->state = DEAD;
    unlock_interrupt();
    schedule();
}

void schedule_callback(char *message) {
    // uart_puts(message);
    lock_interrupt();
    uint64 freq = get_timer_frequency();
    add_timer(schedule_callback, freq >> 5, message);
    unlock_interrupt();
}