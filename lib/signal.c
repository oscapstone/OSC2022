#include "signal.h"
#include "type.h"
#include "interrupt.h"
#include "exception.h"
#include "thread.h"
#include "uart.h"

void signal_default_handlder() {
    kill(currThread->signal_pair.pid);
}

void signal_register(int code, void (*handler)()) {
    if(code <= SIGMAX && code >= 0) {
        currThread->signal_handlers[code] = handler;
    }
}

void signal_kill(uint64 spsr_el1, int pid, int code) {
    lock_interrupt();
    if(pid <= PIDMAX && pid >= 0 && threads[pid].state == USED && code <= SIGMAX && code >= 0) {
        threads[pid].signal_pair.spsr_el1 = spsr_el1;
        threads[pid].signal_pair.pid = pid;
        threads[pid].signal_pair.code = code;
        threads[pid].has_signal = 1;
    }
    unlock_interrupt();
}

void signal_execute() {
    if(currThread->has_signal) {
        // uart_puts("Execute thread: "); uart_num(currThread->pid); uart_puts(", to thread: ");
        // uart_num(currThread->signal_pair.pid); uart_newline();
        currThread->has_signal = 0;
        uint64 spsr_el1 = currThread->signal_pair.spsr_el1;
        int pid = currThread->pid;
        int code = currThread->signal_pair.code;

        if(threads[pid].signal_handlers[code] == signal_default_handlder) {
            signal_default_handlder();
        }
        else {
            lock_interrupt();
            char *signal_user_stack = malloc(THREAD_STACK_SIZE);
            int i = 0;
            read_context(&currThread->signal_context);
            unlock_interrupt();
            
            if(i == 0) {
                i++;
                asm("msr elr_el1, %0\n\t"
                    "msr sp_el0, %1\n\t"
                    "msr spsr_el1, %2\n\t"
                    "eret\n\t" ::"r"(signal_user_mode),
                    "r"(signal_user_stack + THREAD_STACK_SIZE),
                    "r"(spsr_el1));
            }
        }
    }
}

void signal_user_mode() {
    (threads[currThread->signal_pair.pid].signal_handlers[currThread->signal_pair.code])();
    asm("mov x8, 115 \n\t"
        "svc 0\n\t");
}

void signal_return(trapFrame_t *frame) {
    uint64 signal_ustack = frame->sp_el0 - THREAD_STACK_SIZE;
    free(signal_ustack);
    write_context(&currThread->signal_context);
}
