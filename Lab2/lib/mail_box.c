#include "peripherals/mail_box.h"
#include "mail_box.h"
#include "mini_uart.h"
#include "utils.h"

volatile unsigned int  __attribute__((aligned(16))) mailbox[8];

int mailbox_call() {
    /* 
    1. Combine the message address (upper 28 bits) with channel number (lower 4 bits)
    2. Check if Mailbox 0 status register’s full flag is set.
    3. If not, then you can write to Mailbox 1 Read/Write register.
    4. Check if Mailbox 0 status register’s empty flag is set.
    5. If not, then you can read from Mailbox 0 Read/Write register.
    6. Check if the value is the same as you wrote in step 1.
    */
    unsigned int msg = (((unsigned int)((unsigned long)mailbox) & ~0xF) | (0x8 & 0xF));
    while (*MAILBOX_STATUS & MAILBOX_FULL) {}
    *MAILBOX_WRITE = msg;
    while (1) {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) {}
        if(msg == *MAILBOX_READ){
            return mailbox[1] == REQUEST_SUCCEED;
        }
    }
    return 0;
}

void get_board_revision() {
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION; // tag identifier
    mailbox[3] = 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;
    mailbox_call();
    char buffer[20];
    uintoa(buffer, mailbox[5]);
    uart_send_string("\rboard revision: 0x");
    uart_send_string(buffer);  // it should be 0xa020d3 for rpi3 b+
    uart_send_string("\r\n");
}

void get_arm_memory() {
    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_ARM_EMEORY; // tag identifier
    mailbox[3] = 8; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    mailbox[6] = 0; // value buffer
    // tags end
    mailbox[7] = END_TAG;
    mailbox_call();
    char buffer[20];
    uintoa(buffer, mailbox[5]);
    uart_send_string("\rbase address: ");
    uart_send_string(buffer);  // it should be 0x3F000000 for rpi3 b+
    uart_send_string("\r\n");
    uintoa(buffer, mailbox[6]);
    uart_send_string("\rmemory size: ");
    uart_send_string(buffer);  // it should be  for rpi3 b+
    uart_send_string("\r\n");
}