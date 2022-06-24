#include <mailbox.h>
#include <mmio.h>
#include <uart.h>
#include <sched.h>
#include <kmalloc.h>
#include <string.h>
#include <error.h>
#include <mmu.h>

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

int mailbox_request(unsigned char ch, unsigned int* mailbox)
{
    uart_print("mailbox_request(): ch=0x");
    uart_print_hex((uint64_t)ch);
    uart_print(", message addr=0x");
    uart_putshex((uint64_t)mailbox);
    unsigned int *message = (unsigned int *)kmalloc((size_t)mailbox[0]);
    memcpy(message, mailbox, (size_t)mailbox[0]);
    uart_print("mailbox_request(): copy request message to buffer 0x");
    uart_putshex((uint64_t)message);
    unsigned int mailbox_addr = kernel_va_to_pa((unsigned int)((unsigned long long)message & 0xfffffff0));
    //mailbox_addr |= 8;
    mailbox_addr |= ch&0xf;
    uart_puts("mailbox_request(): Wait mailbox not full.");
    while((mmio_load(MAILBOX_STATUS) & MAILBOX_FULL))schedule();
    mmio_set(MAILBOX_WRITE, mailbox_addr);
    uart_puts("mailbox_request(): Wait mailbox not empty.");
    while(1){
        while((mmio_load(MAILBOX_STATUS) & MAILBOX_EMPTY))schedule();
        unsigned int data = mmio_load(MAILBOX_READ);
        if((unsigned char)(data & 0xf) != ch) continue;
        if(data == mailbox_addr){
            memcpy(mailbox, message, (size_t)message[0]);
            kmsg("Mailbox read success.");
            return 1;
        }
        else{
            uart_puts("Mailbox read failed.");
            return 0;
        }
    }
    
    return 0;
}
