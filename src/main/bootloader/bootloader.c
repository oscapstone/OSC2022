#include "uart.h"
#include "type.h"
#include "tty.h"
#include "string.h"

#define MAGIC_CMD   0xdeadbeef

#define SEND_REQ    (MAGIC_CMD ^ 0x1)
#define ACK         (MAGIC_CMD ^ 0x2)

int load_img(uint32_t* start_addr);

extern uint32_t endkernel;


int main(void) {

    init_uart(270);
    uint32_t entry_point = 0x92000;



    if(load_img((uint32_t*)0x92000) == 0) {
        ((void (*)(void))entry_point) ();
    }
    return 0;
}

int load_img(uint32_t* start_addr) {

    ttybuf_t cmd;
    ttybuf_t ksize;
    // uart_write("wait user to send kernel image\n");

    while(1) {
        memset(&cmd, 0, sizeof(cmd));
        tty_recv(&cmd, 4);
        if (cmd.val32 == SEND_REQ) {
            memset(&cmd, 0, sizeof(cmd));
            cmd.val32 = ACK;
            tty_send(&cmd, 4);
            break;
        } else {
            cmd.val32 = 0;
        }
    }

    //* start to load kernel

    //* kernel size

    memset(&cmd, 0, sizeof(cmd));
    tty_recv(&ksize, 8);

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

    return -1;
}