#include <syscall.h>
#include <sched.h>
#include <read.h>
#include <uart.h>
#include <list.h>
#include <irq.h>

extern Thread *run_thread_head;

/* 
 * Return value is x0
 * https://developer.arm.com/documentation/102374/0101/Procedure-Call-Standard
 */


/* Get current process’s id. */
void sys_getpid(TrapFrame *trapFrame){
    int pid = do_getpid();
    trapFrame->x[0] = pid;
}
int do_getpid(){
    return get_current()->id;
}

/* Return the number of bytes read by reading size byte into the user-supplied buffer buf. */
void sys_uart_read(TrapFrame *trapFrame){
    char *buf = (char *)trapFrame->x[0];
    unsigned int size = trapFrame->x[1];
    int idx = readnbyte(buf, size);
    trapFrame->x[0] = idx;
}

/* Return the number of bytes written after writing size byte from the user-supplied buffer buf. */
void sys_uart_write(TrapFrame *trapFrame){
    const char *buf = (char *)trapFrame->x[0];
    unsigned int size = trapFrame->x[1];
    for(unsigned int i = 0; i < size; i++){
        uart_putc(buf[i]);
    }
    trapFrame->x[0] = size;
}
void sys_exec(TrapFrame *trapFrame){

}
void sys_fork(TrapFrame *trapFrame){

}
void sys_exit(TrapFrame *trapFrame){
    do_exit();
}
/* Terminate the current process. */
void do_exit(){
    Thread *exit_thread = get_current();
    exit_thread->state = EXIT;
    list_del(&exit_thread->list);
    Thread *next_thread = (Thread *)run_thread_head->list.next;
    
    enable_irq();
    cpu_switch_to(exit_thread, next_thread);
}

void sys_mbox_call(TrapFrame *trapFrame){

}
void sys_kill(TrapFrame *trapFrame){

}