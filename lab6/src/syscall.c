#include "syscall.h"
#include "sched.h"
#include "stddef.h"
#include "uart.h"
#include "filesystem.h"
#include "exception.h"
#include "malloc.h"
#include "mbox.h"
#include "signal.h"
#include "mmu.h"
#include "string.h"

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
        lock();
        uart_putc(buf[i]);
        unlock();
    }
    tpf->x0 = i;
    return i;
}

//In this lab, you won’t have to deal with argument passing
int exec(trapframe_t *tpf,const char *name, char *const argv[])
{
    kfree(curr_thread->data);
    curr_thread->datasize = get_file_size((char *)name);
    char *new_data = get_file_start((char *)name);
    curr_thread->data = kmalloc(curr_thread->datasize);

    //remap code
    mappages(PHYS_TO_VIRT(curr_thread->context.ttbr0_el1), 0x0, curr_thread->datasize, (size_t)VIRT_TO_PHYS(curr_thread->data), 0);

    for (unsigned int i = 0; i < curr_thread->datasize; i++)
    {
        curr_thread->data[i] = new_data[i];
    }

    //clear signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++)
    {
        curr_thread->singal_handler[i] = signal_default_handler;
    }

    tpf->elr_el1 = 0;
    tpf->sp_el0 = 0xfffffffff000;
    tpf->x0 = 0;
    return 0;
}

int fork(trapframe_t *tpf)
{
    lock();
    thread_t *newt = thread_create(curr_thread->data,curr_thread->datasize);

    //copy signal handler
    for (int i = 0; i <= SIGNAL_MAX;i++)
    {
        newt->singal_handler[i] = curr_thread->singal_handler[i];
    }

    mappages(newt->context.ttbr0_el1, 0x3C000000L, 0x3000000L, 0x3C000000L,0);
    mappages(newt->context.ttbr0_el1, 0x3F000000L, 0x1000000L, 0x3F000000L,0);
    mappages(newt->context.ttbr0_el1, 0x40000000L, 0x40000000L, 0x40000000L,0);

    // remap code and stack
    mappages(newt->context.ttbr0_el1, 0xffffffffb000, 0x4000, (size_t)VIRT_TO_PHYS(newt->stack_alloced_ptr),0);
    mappages(newt->context.ttbr0_el1, 0x0, newt->datasize, (size_t)VIRT_TO_PHYS(newt->data),0);

    // for signal wrapper
    mappages(newt->context.ttbr0_el1, USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED, 0x2000, (size_t)VIRT_TO_PHYS(signal_handler_wrapper), PD_RDONLY);

    //在這段page被蓋爁
    int parent_pid = curr_thread->pid;

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

    //在這段page被蓋爁

    store_context(get_current());
    //for child
    if( parent_pid != curr_thread->pid)
    {
        goto child;
    }

    void *temp_ttbr0_el1 = newt->context.ttbr0_el1;
    newt->context = curr_thread->context;
    newt->context.ttbr0_el1 = VIRT_TO_PHYS(temp_ttbr0_el1);
    newt->context.fp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr; // move fp
    newt->context.sp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr; // move kernel sp

    unlock();

    tpf->x0 = newt->pid;
    return newt->pid;

child:
    tpf->x0 = 0;
    return 0;
}

void exit(trapframe_t *tpf, int status)
{
    thread_exit();
}

int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox_user)
{
    //mbox_user = (unsigned int *)(curr_thread->stack_alloced_ptr + USTACK_SIZE - (0xfffffffff000 - (size_t)mbox));
    lock();

    unsigned int size_of_mbox = mbox_user[0];
    memcpy((char *)mbox, mbox_user, size_of_mbox);
    mbox_call(ch);
    memcpy(mbox_user, (char *)mbox, size_of_mbox);

    tpf->x0 = 8;
    unlock();
    return 0;
}

void kill(trapframe_t *tpf,int pid)
{
    lock();
    if (pid >= PIDMAX || pid < 0  || !threads[pid].isused)
    {
        unlock();
        return;
    }
    threads[pid].iszombie = 1;
    unlock();
    schedule();
}

void signal_register(int signal, void (*handler)())
{
    if (signal > SIGNAL_MAX || signal < 0)return;

    curr_thread->singal_handler[signal] = handler;
}

void signal_kill(int pid, int signal)
{
    if (pid > PIDMAX || pid < 0 || !threads[pid].isused)return;

    lock();
    threads[pid].sigcount[signal]++;
    unlock();
}

void sigreturn(trapframe_t *tpf)
{
    load_context(&curr_thread->signal_saved_context);
}
