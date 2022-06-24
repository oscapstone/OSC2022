#include <syscall.h>
#include <process.h>
#include <stdint.h>
#include <uart.h>
#include <mailbox.h>
#include <signal.h>

void syscall_handler(TrapFrame *trapframe)
{
    uint64_t syscall_num = trapframe->x8;
    //uart_print("syscall_handler(): 0x");
    //uart_putshex(syscall_num);
    switch(syscall_num){
        case 0: // int getpid()
            trapframe->x0 = (uint64_t)process_getpid();
            break;
        case 1: // size_t uartread(char buf[], size_t size)
            trapframe->x0 = (uint64_t)uart_read(trapframe->x0, trapframe->x1);
            break;
        case 2: // size_t uartwrite(const char buf[], size_t size)
            trapframe->x0 = (uint64_t)uart_write(trapframe->x0, trapframe->x1);
            break;
        case 3: // int exec(const char *name, char *const argv[])
            trapframe->x0 = (uint64_t)process_exec(trapframe->x0);
            break;
        case 4: // int fork()
            trapframe->x0 = (uint64_t)process_fork(trapframe);
            break;
        case 5: // void exit(int status)
            process_exit(trapframe->x0);
            break;
        case 6: // int mbox_call(unsigned char ch, unsigned int *mbox)
            trapframe->x0 = (uint64_t)mailbox_request(trapframe->x0, trapframe->x1);
            break;
        case 7: // void kill(int pid)
            process_kill(trapframe->x0);
            break;
        case 8: // signal(int SIGNAL, void (*handler)())
            signal_handler(trapframe->x0, trapframe->x1);
            break;
        case 9: // sigkill(int pid, int SIGNAL)
            signal_kill(trapframe->x0, trapframe->x1);
            break;
        case 10: // sigreturn()
            trapframe->x0 = (uint64_t)mmap(trapframe->x0, trapframe->x1, trapframe->x2, trapframe->x3);
            break;
        case 15: // sigreturn()
            signal_sigreturn();
            break;
    }
}