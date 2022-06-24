#include <error.h>
#include <uart.h>
#include <stdarg.h>
#include <sched.h>
#include <string.h>

void kmsg_(const char *file, int line, const char *function, char *message, ...)
{
    // uart_print("[KERNEL MESSAGE] (");
    // uart_print(file);
    // uart_print(":");
    // uart_print_dec(line);
    // uart_print(") ");
    // uart_puts(message);
    char buf[0x480];
    char buf2[0x400];
    //uart_print(strformat())
    va_list ap;
    va_start(ap, message);
    uart_print(strformat(buf, 0x480, "[KERNEL MESSAGE] (%s() @ %s:%d) %s\n", function, file, line, (char *)vstrformat(buf2, 0x400, message, ap)));
    va_end(ap);
}

void kpanic_(const char *file, int line, const char *function, char *message, ...)
{
    // uart_print("[KERNEL PANIC!!!] (");
    // uart_print(file);
    // uart_print(":");
    // uart_print_dec(line);
    // uart_print(") ");
    // uart_puts(message);
    char buf[0x480];
    char buf2[0x400];
    //uart_print(strformat())
    va_list ap;
    va_start(ap, message);
    uart_print(strformat(buf, 0x480, "[KERNEL PANIC!!!] (%s() @ %s:%d) %s\n", function, file, line, (char *)vstrformat(buf2, 0x400, message, ap)));
    va_end(ap);
    Thread *thread = get_thread(thread_get_current());
    thread->status = ThreadStatus_zombie;
    //schedule();
    while(1);
}