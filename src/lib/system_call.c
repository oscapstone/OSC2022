#include "system_call.h"

/* Implement system calls */
void sys_get_pid(trapframe_t *tf)
{
    // uart_puts("sys_get_pid id : \t");
    // uart_put_int(get_current()->pid);
    // uart_puts("\r\n");

    unsigned long thread_id = get_current()->pid;
    tf->x[0] = thread_id;
}

void sys_uart_read(trapframe_t *tf)
{
    char* buf = (char*)tf->x[0];
    unsigned long size = tf->x[1];
    for(int i = 0; i < size; i++) 
    {
        buf[i] = uart_getc();
    }
    buf[size] = '\0';
    tf->x[0] = size;
}

void sys_uart_write(trapframe_t *tf) 
{
    const char *buf = (char*)tf->x[0];
    unsigned long size = tf->x[1];
    for (int i = 0; i < size; i++)
    {
        uart_send(buf[i]);
    }
    tf->x[0] = size;
}

void sys_exec(trapframe_t *tf) //ok??
{
    char *name = (char*)tf->x[0];
	char *const argv = (char *const)tf->x[1];
    thread *current = get_current();
    exec_old((cpio_new_header *)CPIO_BASE, name, 0);
    tf->elr_el1 = (unsigned long long)current->program_addr;
    tf->sp_el0 = (unsigned long long)current->context.sp;    
    tf->x[0] = 0;
/*
    char *name = (char*)tf->x[0];
	char *const argv = (char *const)tf->x[1];
    cpio_new_header *header = 0;
    char *prog;

    char *user_stack_top;
    user_stack_top = malloc(0x2000);
    user_stack_top = user_stack_top + 0x2000;

    prog = cpio_load(header, name);

    thread *current = get_current();
    unsigned long file_size = 0x2000;

	current->program_size = file_size; 

    char *new_addr = (char *)0x30000;
	char *new_addr_start = (char *)0x30000;
    while(current->program_size--)
    {
		*new_addr = *prog;
		new_addr++;
		prog++;
	}
    tf->elr_el1 = (unsigned long long)new_addr_start;
    tf->sp_el0 = (unsigned long long)user_stack_top;
    tf->x[0] = 0;
*/
}

void sys_fork(trapframe *tf) 
{
    thread *parent = get_current();
    thread *child = thread_create(NULL);
    int child_pid = child->pid;
    unsigned long user_fp = child->context.fp;
    thread *next = child->next;

    /* copy the task context & kernel stack (including trap frame) of parent to child */
    char* src = (char*)parent;
    char* dst = (char*)child;
    int size = PAGE_SIZE;
    while(size--) 
    {
        *dst = *src;
        src++;
        dst++;
    }

    /* set up the correct value for registers */
    parent->context.sp = (unsigned long)tf;
    if ((unsigned long)child > (unsigned long)parent)
    {
        child->context.sp = parent->context.sp + ((unsigned long)child - (unsigned long)parent);
    }
    else
    {
        child->context.sp = parent->context.sp - ((unsigned long)parent - (unsigned long)child);
    }

    int parent_user_stack_size = (parent->context.fp) - (tf->sp_el0) + 1;
    child->context.fp = (unsigned long)child + PAGE_SIZE - 1;
    child->context.lr = (unsigned long)load_reg;
    child->pid = child_pid;
    child->context.fp = user_fp;
    child->next = next;
    trapframe *child_tf = (trapframe*)(child->context.sp);
    child_tf->sp_el0 = (child->context.fp) - parent_user_stack_size + 1;
    child_tf->x[0] = 0;
    child_tf->x[29] = child->context.fp;
    tf->x[0] = child->pid;

    /* copy the user stack of parent to child */
    char *src_stack = (char*)(tf->sp_el0);
    char *dst_stack = (char*)(child_tf->sp_el0);
    
    while(parent_user_stack_size--) 
    {
        *dst_stack = *src_stack;
        src_stack++;
        dst_stack++;
    }

    uart_put("sys_fork : parent pid :");
    uart_put_int(parent->pid);
    uart_put("\t child pid : ");
    uart_put_int(child->pid);
    uart_puts("\r\n");
}

// void sys_exit() 
// {
//     task_struct *cur_task = get_current();
//     cur_task->state = TERMINATED;
//     pop_task_from_queue(&run_queue, cur_task);
//     push_task_to_queue(&terminated_queue, cur_task);
//     debug_printf("[DEBUG][sys_exit] thread: %d\n", cur_task->id);
// }

// int sys_mbox_call(unsigned char ch, volatile unsigned int *mbox) 
// {
//     debug_printf("[DEBUG][sys_mbox_call]");
//     return mailbox_call(ch, mbox);
// }

// void sys_kill(int pid) 
// {
//     task_struct *task = NULL;
//     if ((task = find_task_by_id(&run_queue, pid)))
//         pop_task_from_queue(&run_queue, task);
//     else if ((task = find_task_by_id(&wait_queue, pid)))
//         pop_task_from_queue(&wait_queue, task);
//     if (task) {
//         task->state = TERMINATED;
//         push_task_to_queue(&terminated_queue, task);
//     }
//     debug_printf("[DEBUG][sys_kill]");
// }