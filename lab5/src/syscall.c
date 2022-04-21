#include "syscall.h"
#include "sched.h"
#include "stddef.h"
#include "uart.h"
#include "filesystem.h"
#include "exception.h"
#include "malloc.h"

int getpid(trapframe_t* tpf)
{
    tpf->x0 = curr_thread->pid;
    return curr_thread->pid;
}

size_t uartread(trapframe_t *tpf,char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size;i++)
    {
        buf[i] = uart_async_getc();
    }
    tpf->x0 = i;
    return i;
}

size_t uartwrite(trapframe_t *tpf,const char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size; i++)
    {
        uart_async_putc(buf[i]);
    }
    tpf->x0 = i;
    return i;
}

//In this lab, you won’t have to deal with argument passing
int exec(trapframe_t *tpf,const char *name, char *const argv[])
{
    curr_thread->datasize = get_file_size((char*)name);
    char *new_data = get_file_start((char *)name);
    for (unsigned int i = 0; i < curr_thread->datasize;i++)
    {
        curr_thread->data[i] = new_data[i];
    }

    // eret to exception level 0
    asm("msr sp_el0, %0\n\t" ::"r"(curr_thread->stack_alloced_ptr+USTACK_SIZE));

    tpf->elr_el1 = (unsigned long)curr_thread->data;
    tpf->x0 = 0;
    return 0;
}

int fork(trapframe_t *tpf)
{
    lock();
    thread_t *newt = thread_create(curr_thread->data);
    newt->data = kmalloc(curr_thread->datasize);
    newt->datasize = curr_thread->datasize;

    //copy data into new process
    for (int i = 0; i < newt->datasize; i++)
    {
        newt->data[i] = curr_thread->data[i];
    }

    //copy user stack into new process
    for (int i = 0; i < USTACK_SIZE; i++)
    {
        newt->stack_alloced_ptr[i] = curr_thread->stack_alloced_ptr[i];
    }

    //copy stack into new process
    for (int i = 0; i < KSTACK_SIZE; i++)
    {
        newt->kernel_stack_alloced_ptr[i] = curr_thread->kernel_stack_alloced_ptr[i];
    }

    store_context(get_current());
    newt->context = curr_thread->context;
    thread_t *parent_thread_t = curr_thread;

    newt->context.fp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr; // move fp
    newt->context.sp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr; // move kernel sp
    newt->context.lr = (unsigned long)&&child;                                                     // move lr
    unlock();

    tpf->x0 = newt->pid;
    return newt->pid;

child:
    tpf += newt->kernel_stack_alloced_ptr - parent_thread_t->kernel_stack_alloced_ptr; // move tpf
    tpf->elr_el1 += newt->data - parent_thread_t->data; // mov elr_el1 (return to userspace lr)
    tpf->sp_el0 += newt->stack_alloced_ptr - parent_thread_t->stack_alloced_ptr;
    tpf->x0 = 0;
    return 0;
}