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
#include "vfs.h"

// void test_path(char *path, char *currPath) {
//     char dirname[200], rootName[200], abspath[200];
//     char *basename = get_dirname(dirname, path);
//     char *leafname = get_rootname(rootName, path);
//     get_abspath(abspath, path, currPath);
    
//     uart_printf("input: %s, %s\n", path, currPath);
//     uart_printf("dirname: %s\n", dirname);
//     uart_printf("basename: %s\n", basename);
//     uart_printf("rootName: %s\n", rootName);
//     uart_printf("leafname: %s\n", leafname);
//     uart_printf("abspath: %s\n", abspath);
//     uart_dem();
// }

int main(void) {
    uart_init();
    uart_getc(); // flush init signal(0xE0)
    uart_puts("Push any key to start ...\n");
    uart_getc();
    uart_puts("Push any key to start ...\n");
    uart_getc();

    initMemoryPages();
    initThreads();
    init_vfs();
    
    enable_interrupt();
    enable_uart_interrupt();
    enable_core_timer();
    set_core_timer_by_second(1);

    uint64 tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));

    uart_dem();
    execThread("/initramfs/vfs1.img");
    return 0;
}