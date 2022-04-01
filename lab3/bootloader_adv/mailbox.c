#include "mailbox.h"
#include "uart.h"
#include "string.h"
#include "gpio.h"

void get_board_revision(){
  unsigned int  __attribute__((aligned(16))) mailbox[7];
  mailbox[0] = 7 * 4; // buffer size in bytes
  mailbox[1] = REQUEST_CODE;
  // tags begin
  mailbox[2] = GET_BOARD_REVISION; // tag identifier
  mailbox[3] = 4; // maximum of request and response value buffer's length.
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0; // value buffer
  // tags end
  mailbox[6] = END_TAG;
  // message passing procedure call, you should implement it following the 6 steps provided above.
  if(mailbox_call(mailbox)){
    char str[8];
    itoa(mailbox[5], str);
    uart_puts("board revision: 0x");
    uart_puts(str); // it should be 0xa020d3 for rpi3 b+
    uart_puts("\n");
  }else{
    uart_puts("fail response\n");
  }
}

void get_arm_memory(){
  unsigned int  __attribute__((aligned(16))) mailbox[8];
  mailbox[0] = 8 * 4; // buffer size in bytes
  mailbox[1] = REQUEST_CODE;
  // tags begin
  mailbox[2] = GET_ARM_MEMORY; // tag identifier
  mailbox[3] = 8; // maximum of request and response value buffer's length.
  mailbox[4] = 8;
  mailbox[5] = 0; // value buffer
  mailbox[6] = 0;
  // tags end
  mailbox[7] = END_TAG;
  // message passing procedure call, you should implement it following the 6 steps provided above.
  if(mailbox_call(mailbox)){
    char str[16];
    itoa(mailbox[5], str);
    uart_puts("ARM memory base address: 0x");
    uart_puts(str); // it should be 0xa020d3 for rpi3 b+
    uart_puts("\n");
    itoa(mailbox[6], str);
    uart_puts("ARM memory size: 0x");
    uart_puts(str); // it should be 0xa020d3 for rpi3 b+
    uart_puts("\n");
  }else{
    uart_puts("fail response\n");
  }
}
/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mailbox_call(unsigned int mailbox[]){
    unsigned char ch = 8;
    unsigned int r = (((unsigned int)((unsigned long)mailbox)&~0xF) | (ch&0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MAILBOX_WRITE = r;
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MAILBOX_READ)
            /* is it a valid successful response? */
            return mailbox[1]==REQUEST_SUCCEED;
    }
    return 0;
}