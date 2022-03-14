#include "uart.h"
#include "type.h"
#include "tty.h"
#include "string.h"

#define MAGIC_CMD   0xdeadbeef

#define SEND_REQ    (MAGIC_CMD ^ 0x1)
#define ACK         (MAGIC_CMD ^ 0x2)

int load_img(uint32_t* start_addr);

volatile uint32_t magic = 0;
extern uint32_t endkernel; //* This mean that symbol
extern uint32_t __dtb_start_addr_; //* The address of dtb file


int main(void) {

    init_uart(270);
    // pll0_uart_init();
    uint32_t entry_point = 0x80000;
    uint32_t relo_addr = 0x40000;
    
    if(magic == 0) {
        magic = MAGIC_CMD;
        uint32_t endaddr = &endkernel;
        memcpy(relo_addr, entry_point, (endaddr) - entry_point);
        ((void (*)(void)) relo_addr)();
    } else {
        if(load_img((uint32_t*)entry_point) == 0) { 
            ((void (*)(uint32_t))entry_point) (__dtb_start_addr_);
            // ((void (*)(void))entry_point)();
        }
    }
    
    return 0;
}

int load_img(uint32_t* start_addr) {

    ttybuf_t cmd;
    ttybuf_t ksize;
    // uart_write("wait user to send kernel image\n")
    uint32_t _cmd = 0;



    // unsigned char sym = pll0_uart_getc();

    while(1) {
        // memset(&cmd, 0, sizeof(cmd));
        // tty_recv(&cmd, 4);
        _cmd = 0 | uart_getc();
        _cmd |= uart_getc() << 8;
        _cmd |= uart_getc() << 16;
        _cmd |= uart_getc() << 24;

        if(_cmd == SEND_REQ) {
            uart_putc(ACK&0xff);
            uart_putc((ACK>>8)&0xff);
            uart_putc((ACK>>16)&0xff);
            uart_putc((ACK>>24)&0xff);
            break;
        } else {
            uart_putc(_cmd&0xff);
            uart_putc((_cmd>>8)&0xff);
            uart_putc((_cmd>>16)&0xff);
            uart_putc((_cmd>>24)&0xff);
        }

        // if (cmd.val32 == SEND_REQ) {
        //     memset(&cmd, 0, sizeof(cmd));
        //     cmd.val32 = ACK;
        //     tty_send(&cmd, 4);
        //     break;
        // } else {
        //     tty_send(&cmd, 4);
        //     cmd.val32 = 0;
        // }
    }

    //* start to load kernel

    //* kernel size

    // memset(&cmd, 0, sizeof(cmd));
    memset(&ksize, 0, sizeof(cmd));
    tty_recv(&ksize, 8);
    tty_send(&ksize, 8);

    char* ptr = (char*)start_addr;
    for(int i=0;i<ksize.val64;i++) {
        *ptr++ = uart_getc();
    }

    memset(&cmd, 0, sizeof(cmd));
    tty_recv(&cmd, 4);

    if(cmd.val32 == MAGIC_CMD){
        memset(&cmd, 0, sizeof(cmd));
        cmd.val32 = ACK;
        tty_send(&cmd, 4);
        return 0;
    }
    cmd.val32 = 0x50505050;
    tty_send(&cmd, 4);
    return -1;
}