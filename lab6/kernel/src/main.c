#include "uart.h"
#include "string.h"
#include "power.h"
#include "mbox.h"
#include "cpio.h"
#include "timer.h"
#include "exception.h"
#include "alloc.h"
#include "utils.h"
#include "thread.h"
#include "printf.h"
#include "shell.h"

// #include "device_tree.h"
int a;
void main()
{
    // set up serial console
    uart_init();
    buddy_init();
    //welcome message
    uart_puts("*****************************\r\n");
    uart_puts("*       welcome OSC2022     *\r\n");
    uart_puts("*****************************\r\n");
    timeout_event_init();
    enable_interrupt();
    thread_init();
    printf("%p\n", &a);
    int b;
    printf("%p\n", &b);

    uint64_t *pmd = (uint64_t *)PMD_BASE;
    uint64_t *pte1 = (uint64_t *)(PTE_BASE + 503 * 0x1000);
    uint64_t *pte2 = (uint64_t *)(PTE_BASE + 504 * 0x1000);
    printf("0x%llx\n", *((uint64_t *)PGD_BASE));
    printf("0x%llx\n", *(pmd + 0));
    printf("0x%llx\n", *(pmd + 1));
    printf("0x%llx\n", *(pte1 + 510));
    printf("0x%llx\n", *(pte1 + 511));
    printf("0x%llx\n", *(pte2 + 0));
    printf("0x%llx\n", *(pte2 + 1));
    run_shell();
}

