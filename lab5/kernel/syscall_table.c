#include "kernel/syscall_table.h"

uint64_t syscall_handler(){
    struct task_struct *current = get_current();
    struct trap_frame* trap_frame = get_trap_frame(current);
    uint64_t NR_syscall = trap_frame->x8;
    uint64_t x0, x1, x2, x3, x4;
    uint64_t ret;

    switch(NR_syscall){
        case 0:
            ret = sys_getpid();
            break;
        case 1:
            x0 = trap_frame->x0;
            x1 = trap_frame->x1;
            ret = sys_uart_read((uint8_t*)x0, x1);
            break;
        case 2:
            x0 = trap_frame->x0;
            x1 = trap_frame->x1;
            ret = sys_uart_write((uint8_t*)x0, x1);
            break;
        case 3:
            x0 = trap_frame->x0;
            x1 = trap_frame->x1;
            ret = sys_exec((const char *)x0, (char **const)x1);
            break;
        case 4:
            ret = sys_fork();
            break;
        case 5:
            x0 = trap_frame->x0;
            sys_exit(x0);
            break;
        case 6:
            x0 = trap_frame->x0;
            x1 = trap_frame->x1;
            ret  = sys_mbox_call(x0, (uint32_t*)x1);
            break;
        case 7:
            x0 = trap_frame->x0;
            sys_kill(x0);
            break;
        case 8:
            x0 = trap_frame->x0;
            x1 = trap_frame->x1;
            sys_signal((int)x0,(void (*)())x1);
            break;
        case 9:
            x0 = trap_frame->x0;
            x0 = trap_frame->x1;
            sys_sigkill((uint64_t)x0,(int)x1);
            break;
        case 10:
            sys_sigreturn();
            break;
        default:
            printf("Unknown system call\r\n");
            while(1);
    }
    return ret;
}
