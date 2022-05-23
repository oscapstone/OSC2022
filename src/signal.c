#include "signal.h"
#include "sched.h"
#include "syscall.h"
#include "allocator.h"
#include "exception.h"
#include "mini_uart.h"
void signal_default_handler(int pid)
{
    sys_kill(0,pid);
}
void signal_registed_handler(void (*handler)())
{
    Thread_struct* cur_thread = get_current();
    cur_thread->run_handler();
    asm volatile
    (
        "mov x8,21\n\t"
        "svc 0\n\t"
    );
}
extern void save_cpu_context(cpu_context* ctx);
void run_signal(trap_frame* tf)
{
    Thread_struct* cur_thread = get_current();
    save_cpu_context(&(cur_thread->saved_context));
    for(int j=0;j<20;j++)
    {
        if(cur_thread->signal_count[j] >0 )
        {
            cur_thread->signal_count[j]--;
            void* sig_func = cur_thread->signal_handler[j];
            if(sig_func == signal_default_handler) // if it is default function, 
            {
                signal_default_handler(cur_thread->id);
            }
            else
            {
                cur_thread->signal_ustack = my_malloc(THREAD_STACK_SIZE);
                cur_thread->run_handler = sig_func;
                asm volatile("mov x0, %0": : "r"(sig_func));
                asm volatile("msr sp_el0, %0" : : "r"(cur_thread->signal_ustack + THREAD_STACK_SIZE));
                asm volatile("msr elr_el1, %0": : "r"(signal_registed_handler));
                // asm volatile("msr spsr_el1, %0" : : "r"(0));
                asm volatile("eret");
            }
            
        }
    }
    
}
extern void load_cpu_context(cpu_context* ctx);
void sigreturn()
{
    disable_interrupt();
    Thread_struct* cur_thread = get_current();
    free(cur_thread->signal_ustack);
    load_cpu_context(&(cur_thread->saved_context));
    enable_interrupt();
}