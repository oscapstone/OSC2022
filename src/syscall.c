#include "syscall.h"
#include "sched.h"
#include "mini_uart.h"
#include "cpio.h"
#include "allocator.h"
#include "utils.h"
extern Thread_struct* get_current();
int sys_getpid(trap_frame* tf){
    tf->x0 = get_current()->id;
    return get_current()->id;
}
unsigned int sys_uart_read(trap_frame* tf,char buf[],unsigned int size)
{
    for (int i = 0; i < size; i++)
    {
        // buf[i] = read_uart();
        do{asm volatile("nop");}while(is_empty_read());
        buf[i] = uart_buf_read_pop();
    }
    tf->x0 = size;
    return size;
}
unsigned int sys_uart_write(trap_frame* tf,const char* name,unsigned int size)
{
    for (int i = 0; i < size; i++)
    {
        // busy_wait_writec(name[i]);
        writec_uart(name[i]);
    }
    tf->x0 = size;
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
        // asm volatile("msr sp_el0, %0" : : "r"(get_current()->user_stack + THREAD_STACK_SIZE));
        // asm volatile("msr elr_el1, %0": : "r"(file_start));
        // asm volatile("msr spsr_el1, %0" : : "r"(SPSR_EL1_VALUE));
        // asm volatile("eret");
    }
    return 0;
}
int sys_fork(trap_frame* tf)
{

}
void sys_exit(trap_frame* tf)
{

}
int sys_mbox_call(trap_frame* tf, unsigned char ch, unsigned int *mbox)
{

}
void sys_kill(trap_frame* tf, int pid)
{
    
}