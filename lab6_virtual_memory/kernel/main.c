#include "uart.h"
#include "power.h"
#include "string.h"
#include "utils.h"
#include "address.h"
#include "cpio.h"
#include "type.h"
#include "memory.h"
#include "timer.h"
#include "interrupt.h"
#include "thread.h"
#include "exception.h"

int main(void) {
    uart_init();
    uart_getc(); // flush init signal(0xE0)
    uart_puts("Push any key to start ...\n");
    uart_getc();
    uart_puts("Push any key to start ...\n");
    uart_getc();

    initMemoryPages();
    initThreads();
    
    enable_interrupt();
    enable_uart_interrupt();
    enable_core_timer();
    set_core_timer_by_second(1);

    uint64 tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));

    char* program_address = load_user_program((char*)INITRAMFS_ADDR, NULL, "vm.img");
    uint64 program_size = get_program_size((char*)INITRAMFS_ADDR, "vm.img");
    uart_printf("Start run user program at 0x%x, with size = %dB\n", program_address, program_size);
    uart_dem();
    execThread(program_address, program_size);

    return 0;
}