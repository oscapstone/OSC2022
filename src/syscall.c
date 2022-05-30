#include "syscall.h"
#include "sched.h"
#include "mini_uart.h"
#include "cpio.h"
#include "allocator.h"
#include "utils.h"
#include "exception.h"
#include "mailbox.h"
#include "timer.h"
extern Thread_struct* get_current();
int sys_getpid(trap_frame* tf){
    disable_interrupt();
    int pid = get_current()->id;
    tf->x0 = pid;
    // unsigned long long reg_sp_el0;
    // asm volatile(
    //     "mrs %0,sp_el0\n\t"
    //     : "=r" (reg_sp_el0)
    // );
    // busy_wait_writehex(reg_sp_el0,TRUE);
    enable_interrupt();
    return pid;
}
unsigned int sys_uart_read(trap_frame* tf,char buf[],unsigned long long size)
{
    enable_interrupt();
    for (int i = 0; i < size; i++)
    {
        // buf[i] = read_uart();
        do{asm volatile("nop");}while(is_empty_read());
        buf[i] = uart_buf_read_pop();
    }
    disable_interrupt();
    tf->x0 = size;
    enable_interrupt();
    return size;
}
unsigned int sys_uart_write(trap_frame* tf,const char* name,unsigned long long size)
{
    enable_interrupt();
    for (int i = 0; i < size; i++)
    {
        busy_wait_writec(name[i]);
        // writec_uart(name[i]);
    }
    disable_interrupt();
    tf->x0 = size;
    enable_interrupt();
    return size;
}
int kernel_exec(trap_frame* tf,const char* name, char *const argv[])
{
    // char *file_start = nullptr;
    // unsigned long filesize;
    // cpio_get_addr(file_start,&filesize);
    // char *new_start = my_malloc(filesize);
    // memcpy(new_start,file_start,filesize);
    // thread_exec(new_start);


    // if(filesize!=0){ // file not found if filesize == 0
    //     tf->sp_el0 = (unsigned long)(get_current()->user_stack + THREAD_STACK_SIZE);
    //     tf->elr_el1 = (unsigned long)new_start;
    //     tf->spsr_el1 = 0;
    //     // asm volatile("msr sp_el0, %0" : : "r"(get_current()->user_stack + THREAD_STACK_SIZE));
    //     // asm volatile("msr elr_el1, %0": : "r"(file_start));
    //     // asm volatile("msr spsr_el1, %0" : : "r"(SPSR_EL1_VALUE));
    //     // asm volatile("eret");
    // }
    return 0;
}
int sys_exec(trap_frame* tf,const char* name, char *const argv[])
{
    char **file_start = my_malloc(sizeof(char*));
    unsigned long* filesize = my_malloc(sizeof(unsigned long));
    cpio_get_addr(file_start,filesize);
    char *new_start = my_malloc(*filesize);
    memcpy(new_start,*file_start,*filesize);
    if(filesize!=0){ // file not found if filesize == 0
        tf->sp_el0 = (unsigned long)(get_current()->user_stack + THREAD_STACK_SIZE);
        tf->elr_el1 = (unsigned long)new_start;
        tf->spsr_el1 = 0;
    }
    return 0;
}
extern void load_eret();
int sys_fork(trap_frame* tf)
{
    disable_interrupt();
    // busy_wait_writehex(tf->sp_el0,TRUE);
    Thread_struct* cur_thread = get_current();

    int child_id = thread_create((void*)cur_thread->cpu_context.lr);
    
    Thread_struct* child_thread = get_thread(child_id);

    for(int i=0;i<20;i++)
    {
        child_thread->signal_handler[i] = cur_thread->signal_handler[i];
    }
    // // copy parent's cpu context to child
    memcpy(&(child_thread->cpu_context),&(cur_thread->cpu_context),sizeof(cpu_context));
    // // copy parent's user stack to child
    memcpy(child_thread->user_stack,cur_thread->user_stack,THREAD_STACK_SIZE);
    // // copy trap frame to child sp
    trap_frame* child_tf = (trap_frame*)(child_thread->kernel_stack + THREAD_STACK_SIZE-sizeof(trap_frame));
    memcpy((void*)(child_tf),(void*)tf,sizeof(trap_frame));
    child_tf->x0 = 0;
    child_tf->sp_el0 = (unsigned long long)(child_thread->user_stack+THREAD_STACK_SIZE) - (unsigned long long)((unsigned long long)cur_thread->user_stack+THREAD_STACK_SIZE-tf->sp_el0);
    
    child_thread->cpu_context.sp = (unsigned long long)child_tf;
    child_thread->cpu_context.lr = (unsigned long long)(&load_eret);
    tf->x0 = child_id;
    
    enable_interrupt();
    return child_id;
}
void sys_exit(trap_frame* tf)
{
    disable_interrupt();
    get_current()->state = ZOMBIE;
    enable_interrupt();
    schedule();
}
int sys_mbox_call(trap_frame* tf, unsigned char ch, unsigned int *mbox)
{
    disable_interrupt();

    int res = mailbox_call(mbox,ch);
    tf->x0 = res;
    return res;
    enable_interrupt();
}
void sys_kill(trap_frame* tf, int pid)
{
    disable_interrupt();
    kill_thread(pid);
    enable_interrupt();
}

void signal_register(int signal,void (*handler)())
{
    disable_interrupt();
    if(!(signal<0 || signal>20))
        get_current()->signal_handler[signal]=handler;
    enable_interrupt();
}
void signal_kill(int pid,int signal)
{
    disable_interrupt();
    raise_signal( pid, signal);
    enable_interrupt();
}