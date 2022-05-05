#include "syscall.h"

// syscall no 0
int getpid(trapframe_t *tpf) {
    tpf->x0 = curr_thread->pid;
    return curr_thread->pid;
}

// syscall no 1
unsigned long uartread(trapframe_t *tpf, char buf[], unsigned long size) {
    int i = 0;
    for (int i = 0; i < size; i++)
        buf[i] = uart_read_char_async();
    tpf->x0 = i;
    return i;
}

// syscall no 2
unsigned long uartwrite(trapframe_t *tpf, const char buf[], unsigned long size) {
    int i = 0;
    for (int i = 0; i < size; i++)
        uart_write_char_async(buf[i]);
    tpf->x0 = i;
    return i;
}

// syscall no 3
int exec(trapframe_t *tpf, const char *name, char *const argv[]) {
    curr_thread->datasize = get_file_size((char*)name);
    char *new_data = get_file_start((char *)name);
    // copy data
    for (unsigned int i = 0; i < curr_thread->datasize; i++)
        curr_thread->data[i] = new_data[i];
    // clear signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++)
        curr_thread->singal_handler[i] = signal_default_handler;
    // set return address, stack pointer and return value
    tpf->elr_el1 = (unsigned long)curr_thread->data;
    tpf->sp_el0 = (unsigned long)curr_thread->stack_alloced_ptr + USTACK_SIZE;
    tpf->x0 = 0;
    return 0;
}

// syscall no 4
int fork(trapframe_t *tpf) {
    lock();
    thread_t *newt = thread_create(curr_thread->data);

    // copy signal handler
    for (int i = 0; i <= SIGNAL_MAX; i++)
        newt->singal_handler[i] = curr_thread->singal_handler[i];
    
    newt->datasize = curr_thread->datasize;
    int parent_pid = curr_thread->pid;
    thread_t *parent_thread_t = curr_thread;

    // can not copy data because there are a lot of ret addresses on stack
    // copy user stack into new process
    for (int i = 0; i < USTACK_SIZE; i++)
        newt->stack_alloced_ptr[i] = curr_thread->stack_alloced_ptr[i];
    // copy kernel stack into new process
    for (int i = 0; i < KSTACK_SIZE; i++)
        newt->kernel_stack_alloced_ptr[i] = curr_thread->kernel_stack_alloced_ptr[i];

    store_context(get_current());
    // child
    if (parent_pid != curr_thread->pid) {
        // move trapframe
        tpf = (trapframe_t*)((char *)tpf + (unsigned long)newt->kernel_stack_alloced_ptr - (unsigned long)parent_thread_t->kernel_stack_alloced_ptr);
        tpf->sp_el0 += newt->stack_alloced_ptr - parent_thread_t->stack_alloced_ptr;
        tpf->x0 = 0;
        return 0;
    }
    // parent
    else {
        newt->context = curr_thread->context;
        // move fp
        newt->context.fp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr;
        // move kernel sp
        newt->context.sp += newt->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr;

        unlock();

        tpf->x0 = newt->pid;
        return newt->pid;
    }    
}

// syscall no 5
void exit(trapframe_t *tpf, int status) {
    thread_exit();
}

// syscall no 6
int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox) {
    lock();
    unsigned int req = (((unsigned int)((unsigned long)mbox) & (~0xF)) | (ch & 0xF));
    do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_FULL);
    *MAILBOX_WRITE = req;
    while (1) {
        do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_EMPTY);
        if(req == *MAILBOX_READ) {
            tpf->x0 = (mbox[1] == MAILBOX_RESPONSE);
            unlock();
            return mbox[1] == MAILBOX_RESPONSE;
        }
    }
    tpf->x0 = 0;
    unlock();
    return 0;
}

// syscall no 7
void kill(trapframe_t *tpf, int pid) {
    lock();
    // check for invalid pid
    if (pid >= PIDMAX || pid < 0 || !threads[pid].isused) {
        unlock();
        return;
    }

    threads[pid].iszombie = 1;
    unlock();
    schedule();
}

// syscall no 8
void signal_register(int signal, void (*handler)()) {
    // invalid signal
    if (signal > SIGNAL_MAX || signal < 0)
        return;
    curr_thread->singal_handler[signal] = handler;
}

// syscall no 9
void signal_kill(int pid, int signal) {
    // check for invalid pid
    if (pid >= PIDMAX || pid < 0 || !threads[pid].isused)
        return;
    lock();
    threads[pid].sigcount[signal]++;
    unlock();
}

// syscall no 15
void sigreturn(trapframe_t *tpf) {
    unsigned long signal_ustack;
    if (tpf->sp_el0 % USTACK_SIZE == 0)
        signal_ustack = tpf->sp_el0 - USTACK_SIZE;
    else
        signal_ustack = tpf->sp_el0 & (~(USTACK_SIZE - 1));
    free_page((char*)signal_ustack);
    load_context(&curr_thread->signal_saved_context);
}
