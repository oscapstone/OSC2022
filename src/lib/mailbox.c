#include <mailbox.h>
#include <mmio.h>
#include <uart.h>

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

int mailbox_request(unsigned int* mailbox)
{
    unsigned int mailbox_addr = (unsigned int)((unsigned long long)mailbox & 0xfffffff0);
    mailbox_addr |= 8;
    uart_puts("Wait mailbox not full.");
    while((mmio_load(MAILBOX_STATUS) & MAILBOX_FULL));
    mmio_set(MAILBOX_WRITE, mailbox_addr);
    uart_puts("Wait mailbox not empty.");
    while(1){
        while((mmio_load(MAILBOX_STATUS) & MAILBOX_EMPTY));
        unsigned int data = mmio_load(MAILBOX_READ);
        if((data & 0xf) != 8) continue;
        if(data == mailbox_addr){
            return 0;
        }
        else{
            uart_puts("Mailbox read failed.");
            return -1;
        }
    }
    
    return -1;
}
