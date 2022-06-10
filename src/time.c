#include "time.h"

void set_start_time()
{
    register cur_time;
    asm volatile ("mrs %0, cntpct_el0": "=r"(cur_time));
    start_time = cur_time;
}

void print_sec()
{
    uint32_t freq;
    uint64_t cur_time;
    uint64_t sec;
    asm volatile ("mrs %0, cntfrq_el0": "=r"(freq));
    asm volatile ("mrs %0, cntpct_el0": "=r"(cur_time));

    sec = (cur_time - start_time) / freq;
    // sync_uart_puts("\nHas booted after ");
    // uart_dec(sec);
    // sync_uart_puts(" seconds ago\n");

}
