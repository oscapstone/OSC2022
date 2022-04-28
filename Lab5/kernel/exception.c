#include "utils.h"
#include "mini_uart.h"
#include "peripherals/mini_uart.h"
#include "exception.h"
#include "sysreg.h"
#include "timer.h"
#include "system_call.h"
#include "switch.h"
#include "memory.h"
#include "cpio.h"
#include "shell.h"
#include "mail_box.h"


void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }
void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }

/* system call */
void lower_sync_handler(trap_frame *tf) {
    unsigned long esr, svc;
    asm volatile("mrs %0, esr_el1  \n":"=r"(esr):);
	unsigned long *regs = tf->regs;
    if (((esr >> 26) & 0x3f) == 0x15) {
        svc = esr & 0x1ffffff;
        if (svc == 0) {
            
            switch(regs[8]) {
                case 0:
                    regs[0] = sys_getpid();
                    thread_schedule();
                    break;
                case 1:
                    regs[0] = sys_uartread((char*)regs[0], (size_t)regs[1]);
                    break;
                case 2:
                    regs[0] = sys_uartwrite((const char*)regs[0], (size_t)regs[1]);
                    break;
                case 3:
                    sys_exec(tf, (const char*)regs[0], (char * const*)regs[1]);
                    thread_schedule();
                    break;
                case 4:
                    sys_fork(tf);
                    thread_schedule();
                    break;
                case 5:
                    sys_exit(regs[0]);
                    thread_schedule();
                    break;
                case 6:
                    regs[0] = sys_mbox_call((unsigned char)regs[0], (volatile unsigned int*)regs[1]);
                    thread_schedule();
                    break;
                case 7:
                    sys_kill(regs[0]);
                    thread_schedule();
                    break;
                default:
                    uart_printf("[ERROR][lower_sync_handler] unknown svc!\n");
                    break;
            }
        }
        else
            uart_printf("[ERROR][lower_sync_handler] unknown exception!\n");
    }
}

void lower_iqr_handler() {
	pop_timer();
}

void curr_sync_handler() {
    uart_printf("[ERROR][curr_sync_handler]\n");
	error_handler();
}

void curr_iqr_handler() {
    if (*IRQ_PENDING_1 & AUX_IRQ)
		uart_handler();
	else
		pop_timer();
}

void error_handler() {
	uart_send_string("[ERROR] unknown exception...\n");
	while(1){}
}

void dumpState() {
	unsigned long esr = read_sysreg(esr_el1);
	unsigned long elr = read_sysreg(elr_el1);
	unsigned long spsr = read_sysreg(spsr_el1);
	uart_printf("--------------------\n");
	uart_printf("SPSR: 0x%x\n", spsr);
	uart_printf("ELR: 0x%x\n", elr);
	uart_printf("ESR: 0x%x\n", esr);
	uart_printf("--------------------\n");
}

/* Implement system calls */
int sys_getpid() {
    debug_printf("[DEBUG][sys_getpid] id: %d\n", get_current()->id);
    return get_current()->id;
}

size_t sys_uartread(char buf[], size_t size) {
    char recv;
    for (int i = 0; i < size; ++i) {
        recv = uart_recv();
        buf[i] = recv;
    }
    debug_printf("[DEBUG][sys_uartread]\n");
    return size;
}

size_t sys_uartwrite(const char buf[], size_t size) {
    for (int i = 0; i < size; ++i)
		uart_send((char)buf[i]);
    debug_printf("[DEBUG][sys_uartwrite]\n");
    return size;
}

int sys_exec(trap_frame *tf, const char *name, char *const argv[]) {
    load_program((char*)name);
    _argv = (char**)argv;
    task_struct *cur_task = get_current();
    tf->elr_el1 = (unsigned long)USER_PROGRAM_ADDR;
    tf->sp_el0 = cur_task->user_fp;
    return 0;
}

void sys_fork(trap_frame *tf) {
    task_struct *parent = get_current();
    task_struct *child = thread_create(NULL);
    int child_id = child->id;
    unsigned long user_fp = child->user_fp;
    task_struct *prev = child->prev;
    task_struct *next = child->next;

    /* copy the task context & kernel stack (including trap frame) of parent to child */
    char* src = (char*)parent;
    char* dst = (char*)child;
    int size = PAGE_SIZE_4K;
    while(size--) {
        *dst = *src;
        src++;
        dst++;
    }

    /* set up the correct value for registers */
    parent->context.sp = (unsigned long)tf;
    if ((unsigned long)child > (unsigned long)parent)
        child->context.sp = parent->context.sp + ((unsigned long)child - (unsigned long)parent);
    else
        child->context.sp = parent->context.sp - ((unsigned long)parent - (unsigned long)child);
    int parent_ustack_size = (parent->user_fp) - (tf->sp_el0) + 1;
    child->context.fp = (unsigned long)child + PAGE_SIZE_4K - 1;
    child->context.lr = (unsigned long)child_return_from_fork;
    child->id = child_id;
    child->user_fp = user_fp;
    child->prev = prev;
    child->next = next;
    trap_frame *child_tf = (trap_frame*)(child->context.sp);
    child_tf->sp_el0 = (child->user_fp) - parent_ustack_size + 1;
    child_tf->regs[0] = 0;
    child_tf->regs[29] = child->context.fp;
    tf->regs[0] = child->id;

    /* copy the user stack of parent to child */
    char *src_stack = (char*)(tf->sp_el0);
    char *dst_stack = (char*)(child_tf->sp_el0);
    
    while(parent_ustack_size--) {
        *dst_stack = *src_stack;
        src_stack++;
        dst_stack++;
    }

    debug_printf("[DEBUG][sys_fork] parent: %d, child: %d\n", parent->id, child->id);
}

void sys_exit() {
    task_struct *cur_task = get_current();
    cur_task->state = TERMINATED;
    pop_task_from_queue(&run_queue, cur_task);
    push_task_to_queue(&terminated_queue, cur_task);
    debug_printf("[DEBUG][sys_exit] thread: %d\n", cur_task->id);
}

int sys_mbox_call(unsigned char ch, volatile unsigned int *mbox) {
    debug_printf("[DEBUG][sys_mbox_call]");
    return mailbox_call(ch, mbox);
}

void sys_kill(int pid) {
    task_struct *task = NULL;
    if ((task = find_task_by_id(&run_queue, pid)))
        pop_task_from_queue(&run_queue, task);
    else if ((task = find_task_by_id(&wait_queue, pid)))
        pop_task_from_queue(&wait_queue, task);
    if (task) {
        task->state = TERMINATED;
        push_task_to_queue(&terminated_queue, task);
    }
    debug_printf("[DEBUG][sys_kill]");
}
