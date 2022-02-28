#include <stdint.h>
#include <stddef.h>
#include <uart.h>
#include <string.h>
#include <mmio.h>
#include <utils.h>

#define PM_PASSWORD 0x5a000000

int test[0x10];

extern uint32_t _bss_start;
extern uint32_t _bss_end;
extern uint32_t _stack_end;

void main();

void _init()
{
    for(uint32_t* addr = &_bss_start; addr!=&_bss_end; addr++){
        *addr = 0;
    }
    main();
}

void reset(int tick) {                 // reboot after watchdog timer expire
    mmio_set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    mmio_set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    mmio_set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    mmio_set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}

void help()
{
    uart_puts("help         : print this help menu");
    uart_puts("hello        : print Hello World!");
    uart_puts("reboot       : reboot the device");
}

void main()
{
    char buf[0x10];
    uart_init();

    uart_puts("Boot!!!");
    //uart_puts("Shell");
    unsigned int board_revision;
    void* arm_memory_base;
    unsigned int arm_memory_size;

    get_board_revision(&board_revision);
    get_ARM_memory_info(&arm_memory_base, &arm_memory_size);

    u322hex(board_revision, buf, 0x10);
    uart_print("Board Revision: 0x");
    uart_puts(buf);

    u642hex((unsigned long long)arm_memory_base, buf, 0x10);
    uart_print("ARM Memory Base: 0x");
    uart_puts(buf);
    
    u322hex(arm_memory_size, buf, 0x10);
    uart_print("ARM Memory Size: 0x");
    uart_puts(buf);

    while(1){
        uart_print("# ");
        uart_gets(buf);
        //uart_puts(buf);
        if(strncmp(buf, "help", 4) == 0){
            help();
            //help();
        }
        else if(strncmp(buf, "hello", 5) == 0){
            uart_puts("Hello World!");
        }
        else if(strncmp(buf, "reboot", 5) == 0){
            uart_puts("reboot!!");
            reset(1<<16);
        }
    }
    uart_write(buf, 5);
    //uart_write("Hello, world!", 13);
}