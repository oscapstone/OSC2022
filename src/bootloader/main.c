#include <stdint.h>
#include <stddef.h>
#include <uart.h>
#include <string.h>

extern uint32_t _bootloader_start;
extern uint32_t _code_start;
extern uint32_t _code_end;
extern uint32_t _data_start;
extern uint32_t _data_end;
extern uint32_t _kernel_start;

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

void main()
{
    uart_init();
    uart_print("Kernel Image Size: ");
    char buf[0x20];
    size_t recvlen = uart_gets(buf);
    buf[recvlen] = 0;
    int imagesize = atoi(buf);
    //char* kernel_addr = (char *)&_code_start;
    //kernel_addr += (uint64_t)((uint64_t)&_code_start - (uint64_t)&_bootloader_start);
    uart_print("Input Kernel Image (Size: 0x");
    char sizehexstr[0x20];
    u322hex(imagesize, sizehexstr, 0x1f);
    uart_print(sizehexstr);
    uart_print(") : ");
    uart_read((char *)&_kernel_start, imagesize);
    //goto *&_kernel_start;
}