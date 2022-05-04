#include <signal.h>
#include <sched.h>
#include <process.h>
#include <stdint.h>
#include <kmalloc.h>
#include <interrupt.h>
#include <uart.h>
#define STACK_PAGES 2
extern void signal_call_sigreturn();

SignalHandler *get_signalhandler(Process *process, uint8_t signal)
{
    SignalHandler *cur_h = process->signal_handlers;
    while(cur_h){
        if(cur_h->signum == signal) return cur_h;
        cur_h = cur_h->next;
    }
    return 0;
}

void signal_handler(uint8_t SIGNAL, void (*handler)())
{
    Thread *thread = get_thread(thread_get_current());
    if(!thread) goto err;
    Process *process = thread->process;
    if(!process) goto err;
    if(SIGNAL>SIGNAL_NUM || SIGNAL<1) goto err;
    SignalHandler *sig_h = get_signalhandler(process, SIGNAL);
    if(!sig_h){
        sig_h = (SignalHandler*)kmalloc(sizeof(SignalHandler));
        sig_h->signum = SIGNAL;
        sig_h->next = process->signal_handlers;
        process->signal_handlers = sig_h;
    }
    sig_h->handler = handler;

    err:
    return ;
}

void signal_kill(uint64_t pid, uint8_t SIGNAL)
{
    Process *process = get_process(pid);
    if(!process){
        return ;
    }
    if(SIGNAL>SIGNAL_NUM || SIGNAL<1) return ;
    process->signal[SIGNAL] = 1;
}

void signal_default_handler_9()
{
    //uart_puts("signal_default_handler_9(): default handler for signal 9.");
    asm(
        "mov x0, #1\t\n"
        "mov x8, #5\t\n"
        "svc 0\t\n"
    );
}

void signal_default_handler()
{
    //uart_puts("signal_default_handler(): default signal handler.");
    asm(
        "mov x8, #10\t\n"
        "svc 0\t\n"
    );
}

void signal_check()
{
    Thread *thread = get_thread(thread_get_current());
    Process *process = thread->process;
    if(!process) goto ret;
    if(process->signal_handling) goto ret;
    for(int i=1;i<=SIGNAL_NUM;i++){
        if(process->signal[i]==0) continue;
        interrupt_disable();
        Thread *sighandle_thread = create_thread(signal_handler_exec, (void *)i);
        process->signal_handling = i;
        process->signal_tid = sighandle_thread->thread_id;
        sighandle_thread->process = process;
        process->status = ProcessStatus_wait;
        process->signal[i] = 0;
        thread_run(sighandle_thread);
        wait_thread(thread);
        interrupt_enable();
    }
    ret:
    return ;
}

void signal_handler_exec(uint8_t SIGNAL)
{
    Thread *thread = get_thread(thread_get_current());
    Process *process = thread->process;
    SignalHandler *sighandler = get_signalhandler(process, SIGNAL);
    void (*handler)();
    if(sighandler)handler = sighandler->handler;
    else if(SIGNAL==9) handler = signal_default_handler_9;
    else handler = signal_default_handler;
    process->sigstack = buddy_alloc(STACK_PAGES);

    asm("msr spsr_el1, %0"::"r"((uint64_t)0x0)); // 0x0 enable all interrupt
    asm("msr elr_el1, %0"::"r"(handler));
    asm("msr sp_el0, %0"::"r"((uint64_t)process->sigstack + ((uint64_t)STACK_PAGES << PAGE_SIZE) - 16));
    asm("mov x30, %0"::"r"(signal_call_sigreturn));
    asm(
        "mov x0, #0\t\n"
        "mov x1, #0\t\n"
        "mov x2, #0\t\n"
        "mov x3, #0\t\n"
        "mov x4, #0\t\n"
        "mov x5, #0\t\n"
        "mov x6, #0\t\n"
        "mov x7, #0\t\n"
        "mov x8, #0\t\n"
        "mov x9, #0\t\n"
        "mov x10, #0\t\n"
        "mov x11, #0\t\n"
        "mov x12, #0\t\n"
        "mov x13, #0\t\n"
        "mov x14, #0\t\n"
        "mov x15, #0\t\n"
        "mov x16, #0\t\n"
        "mov x17, #0\t\n"
        "mov x18, #0\t\n"
        "mov x19, #0\t\n"
        "mov x20, #0\t\n"
        "mov x21, #0\t\n"
        "mov x22, #0\t\n"
        "mov x23, #0\t\n"
        "mov x24, #0\t\n"
        "mov x25, #0\t\n"
        "mov x26, #0\t\n"
        "mov x27, #0\t\n"
        "mov x28, #0\t\n"
        "mov x29, #0\t\n"
        //"mov x30, #0\t\n"
    );
    asm("eret");
}

void signal_sigreturn()
{
    interrupt_disable();
    Thread *thread = get_thread(thread_get_current());
    Process *process = thread->process;
    process->signal_handling = 0;
    buddy_free(process->sigstack);
    process->signal_tid = 0;
    process->status = ProcessStatus_running;
    thread->process = 0;
    wakeup_thread(get_thread(process->tid));
    thread->status = ThreadStatus_zombie;
    interrupt_enable();
    schedule();
}