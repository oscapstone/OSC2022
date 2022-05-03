// https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1-

#include "utils.h"
#include "exception.h"
#include "sys.h"
#include "printf.h"
#include "sched.h"
#include "memory.h"
#include "uart.h"
#include "mbox.h"
#include "entry.h"
#include "stdlib.h"
#include "thread.h"
#include "buddy_system.h"
#include <stddef.h>

extern void end_of_thread(void);
extern void ret_from_fork(void);
extern struct task_struct *task[];

void sync_exc_router(unsigned long esr_el1, unsigned long elr_el1, Trapframe *trapframe) {
    int ec = (esr_el1 >> 26) & 0b111111;
    int iss = esr_el1 & 0x1FFFFFF;
    if (ec == 0b010101) { // is system call
        unsigned long syscall_num = trapframe->x[8];
        // printf("[SYSCALL] %d\n", syscall_num);
        syscall(syscall_num, trapframe);
    } else {
        // return;
        printf("Exception return address 0x%x\n", elr_el1);
        printf("Exception class (EC) 0x%x\n", ec);
        printf("Instruction specific syndrome (ISS) 0x%x\n", iss);
    }
}

void syscall(unsigned long syscall_num, Trapframe *trapframe) {
    switch (syscall_num) {
    case SYS_GETPID:
        sys_getpid(trapframe);
        break;

    case SYS_UART_READ:
        sys_uart_read(trapframe);
        break;

    case SYS_UART_WRITE:
        sys_uart_write(trapframe);
        break;

    case SYS_EXEC:
        sys_exec(trapframe);
        break;

    case SYS_FORK:
        sys_fork(trapframe);
        break;

    case SYS_EXIT:
        sys_exit(trapframe);
        break;

    case SYS_MBOX_CALL:
        sys_mbox_call(trapframe);
        break;
    case SYS_KILL:
        sys_kill(trapframe);
        break;
    }
    return;
}

void sys_getpid(Trapframe *trapframe) {
    trapframe->x[0] = current_thread()->pid;
}

void sys_uart_read(Trapframe *trapframe) {
    char *buf = (char *)trapframe->x[0];
    size_t size = (size_t)trapframe->x[1];
    enable_irq(); // 避免卡在 read 裡
    for (int i = 0; i < size; i++) {
        *(buf + i) = uart_getc();
    }
    disable_irq();
    trapframe->x[0] = size;
}

void sys_uart_write(Trapframe *trapframe) {
    const char *buf = (const char *)trapframe->x[0];
    size_t size = (size_t)trapframe->x[1];
    enable_irq();
    for (int i = 0; i < size; i++) {
        uart_send(*(buf + i));
    }
    disable_irq();
    trapframe->x[0] = size;
}

void sys_exec(Trapframe *trapframe) {
    preempt_disable();
    char *input = trapframe->x[0];
    char *program_pos;
    cpio_newc_header *fs = (cpio_newc_header *)0x8000000;
    char *current = (char *)0x8000000;
    int name_size;
    int file_size;
    while (1) {
        fs = (cpio_newc_header *)current;
        name_size = hex_to_int(fs->c_namesize, 8);
        file_size = hex_to_int(fs->c_filesize, 8);
        current += 110;
        if (strcmp(current, "TRAILER!!!") == 0) {
            uart_puts("No such file!\n");
            break;
        }
        if (strcmp(current, input) == 0) {
            current += name_size;
            while ((current - (char *)fs) % 4 != 0)
                current++;
            program_pos = (char *)current;
            break;
        } else {
            current += name_size;
            while ((current - (char *)fs) % 4 != 0)
                current++;
            current += file_size;
            while ((current - (char *)fs) % 4 != 0)
                current++;
        }
    }
    printf("file_size: %d\n", file_size);
    char *new_program_pos = (char *)buddy_system_alloc(file_size / 1024 + 1);
    // char *new_program_pos = (char *)malloc(file_size + 4096);
    for (int i = 0; i < file_size; i++) {
        *(new_program_pos + i) = *(program_pos + i);
    }
    printf("program pos : %x", new_program_pos);
    preempt_enable();
    struct task_struct *cur = current_thread();
    asm volatile("msr sp_el0, %0"
                 :
                 : "r"(cur->user_sp));
    asm volatile("msr elr_el1, %0"
                 :
                 : "r"(new_program_pos));
    asm volatile("msr spsr_el1, %0"
                 :
                 : "r"(0x0));
    asm volatile("eret");
    trapframe->x[0] = 0;
}

void sys_fork(Trapframe *trapframe) {
    struct task_struct *parent = current_thread();
    /*
        ret_from_fork 會把 child_trapframe load to register，
        這樣跑 child thread 時就會用到 child_trapframe 更改的 sp
    */
    int newpid = thread_create(ret_from_fork);

    struct task_struct *child = task[newpid];

    printf("child: %x\n", child);

    // copy kernel stack and user stack
    unsigned long kstack_offset = (char *)parent->kernel_sp - (char *)trapframe;
    unsigned long ustack_offset = (char *)parent->user_sp - (char *)trapframe->sp_el0;

    // copy kernel stack (including trapframe)
    for (unsigned long i = 1; i <= kstack_offset; i++) {
        *((char *)(child->kernel_sp - i)) = *((char *)(parent->kernel_sp - i));
    }

    // copy user stack
    for (unsigned long i = 1; i <= ustack_offset; i++) {
        *((char *)(child->user_sp - i)) = *((char *)(parent->user_sp - i));
    }

    child->cpu_context.sp = child->kernel_sp - kstack_offset;

    Trapframe *child_trapframe = (Trapframe *)child->cpu_context.sp;
    child_trapframe->sp_el0 = child->user_sp - ustack_offset;
    printf("child sp: %x\n", child_trapframe->sp_el0);
    // child_trapframe->spsr_el1 = 0x0; // el0

    trapframe->x[0] = child->pid;
    child_trapframe->x[0] = 0;
}

void sys_exit(Trapframe *trapframe) {
    current_thread()->status = trapframe->x[0];
    end_of_thread();
}

void sys_mbox_call(Trapframe *trapframe) {
    unsigned char ch = (unsigned char)trapframe->x[0];
    unsigned int *mbox = (unsigned int *)trapframe->x[1];
    int ret = mboxc_mbox_call(ch, mbox); // defined in mbox.c
    trapframe->x[0] = ret;
}

void sys_kill(Trapframe *trapframe) {
    task[trapframe->x[0]]->state = ZOMBIE;
}