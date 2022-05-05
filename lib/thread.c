#include "thread.h"
#include "interrupt.h"
#include "memory.h"
#include "list.h"
#include "utils.h"
#include "signal.h"

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

    currThread = idleThread = createThread(idle);
    asm volatile("msr tpidr_el1, %0" ::"r" (&currThread->context));
    unlock_interrupt();
}

thread_t *createThread(void *program) {
    lock_interrupt();
    thread_t *thread;
    for(int i = 0; i <= PIDMAX; i++) {
        if(threads[i].state == IDLE) {
            thread = &threads[i];
            break;
        }
    }

    thread->state = USED;
    thread->context.lr = (uint64)program;
    thread->stackPtr = (char*)malloc(THREAD_STACK_SIZE);
    thread->kernel_stackPtr = (char*)malloc(THREAD_STACK_SIZE);

    thread->context.sp = (uint64)thread->stackPtr + THREAD_STACK_SIZE;
    thread->context.fp = thread->context.sp;
    thread->has_signal = 0;

    list_add(&thread->listHead, run_queue);
    unlock_interrupt();
    return thread;
}

void idle() {
    while(1) {
        kill_zombies();
        // uart_puts("IDLE\n");
        schedule();
    }    
}

void kill_zombies() {
    lock_interrupt();
    list_head_t *list;
    list_for_each(list, run_queue) {
        thread_t *thread = (thread_t*)list;
        if(thread->state == DEAD) {
            // uart_puts("Kill thread "); uart_num(thread->pid); uart_newline();
            list_del_entry(list);
            free(thread->stackPtr);
            free(thread->kernel_stackPtr);
            thread->state = IDLE;
        }
    }
    unlock_interrupt();
}

void schedule() {
    lock_interrupt();
    
    do {
        currThread = (thread_t*)currThread->listHead.next;
    } while(list_is_head(currThread, run_queue) || currThread->state == DEAD);
    
    // uart_puts("Select currThread "); uart_num(currThread->pid); uart_newline();
    switch_to(get_current(), &currThread->context);
    unlock_interrupt();
    // return to context.lr
}

void execThread(char *program, uint64 program_size) {
    lock_interrupt();
    thread_t *thread = createThread(program);

    thread->data = malloc(program_size);
    thread->datasize = program_size;
    thread->context.lr = (uint64)thread->data;
 
    for(int i = 0; i < program_size; i++) {
        thread->data[i] = program[i];
    }

    thread->has_signal = 0;
    for(int i = 0; i <= SIGMAX; i++) {
        thread->signal_handlers[i] = signal_default_handlder;
    }
    
    // uart_puts("Start new thread\n");
    currThread = thread;
    unlock_interrupt();
    set_schedule_timer();
    
    // eret to exception level 0
    asm("msr tpidr_el1, %0\n\t"
        "msr elr_el1, %1\n\t"
        "msr spsr_el1, xzr\n\t" // enable interrupt in EL0. You can do it by setting spsr_el1 to 0 before returning to EL0.
        "msr sp_el0, %2\n\t"
        "mov sp, %3\n\t"
        "eret\n\t" ::"r"(&thread->context),"r"(thread->context.lr), 
                        "r"(thread->context.sp), "r"(thread->kernel_stackPtr + THREAD_STACK_SIZE));
}

void testThread() {
    for(int i = 0; i < 3; ++i) {
        uart_puts("Thread id: "); uart_num(currThread->pid);
        uart_puts(" "); uart_num(i); uart_newline();
        delay_ms(1000);
        schedule();
    }

    thread_exit();
}

void thread_exit() {
    lock_interrupt();
    currThread->state = DEAD;
    unlock_interrupt();
    schedule();
}

void set_schedule_timer() {
    uint64 freq = get_timer_frequency();
    add_timer(schedule_callback, freq >> 5, "Schedule\n");
}

void schedule_callback(char *message) {
    // uart_puts(message);
    set_schedule_timer();
}