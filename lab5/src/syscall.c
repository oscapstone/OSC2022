#include "syscall.h"
#include "sched.h"
#include "stddef.h"
#include "uart.h"
#include "filesystem.h"

int getpid()
{
    return curr_thread->pid;
}

size_t uartread(char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size;i++)
    {
        buf[i] = uart_async_getc();
    }
    return i;
}

size_t uartwrite(const char buf[], size_t size)
{
    int i = 0;
    for (int i = 0; i < size; i++)
    {
        uart_async_putc(buf[i]);
    }
    return i;
}

//In this lab, you won’t have to deal with argument passing
int exec(const char *name, char *const argv[])
{
    curr_thread->datasize = get_file_size((char*)name);
    char *new_data = get_file_start((char *)name);
    for (unsigned int i = 0; i < curr_thread->datasize;i++)
    {
        curr_thread->data[i] = new_data[i];
    }

    __asm__ __volatile__("mov sp, %0\n\t" ::"r"(curr_thread->stack_alloced_ptr + USTACK_SIZE));
    __asm__ __volatile__("msr elr_el1, %0\n\t" ::"r"(curr_thread->data));
    __asm__ __volatile__("eret\n\t");
    return 0;
}