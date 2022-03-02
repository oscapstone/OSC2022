#include "mbox.h"
#include "uart.h"
/* mailbox message buffer */

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
volatile unsigned int  __attribute__((aligned(16))) mbox[36];
int mbox_call(unsigned char ch)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MBOX_READ)
            /* is it a valid successful response? */
            return mbox[1]==MBOX_RESPONSE;
    }
    return 0;
}

void get_board_revision(){
  mbox[0] = 7 * 4; // buffer size in bytes
  mbox[1] = REQUEST_CODE;
  // tags begin
  mbox[2] = GET_BOARD_REVISION; // tag identifier
  mbox[3] = 4; // maximum of request and response value buffer's length.
  mbox[4] = TAG_REQUEST_CODE;
  mbox[5] = 0; // value buffer
  // tags end
  mbox[6] = END_TAG;

  mbox_call(8); // message passing procedure call, you should implement it following the 6 steps provided above.
  uart_puts("0x");
  uart_hex(mbox[5]);
  uart_puts("\r\n");
//   printf("0x%x\n", mbox[5]); // it should be 0xa020d3 for rpi3 b+
}


void mbox_vc_memory() {
    // unsigned int __attribute__((aligned(16))) mbox[8];
    mbox[0] = 8 * 4;  // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = MBOX_TAG_GET_VC_MEMORY;  // tag identifier
    mbox[3] = 8;                       // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;       // tag code
    mbox[5] = 0;                       // base address
    mbox[6] = 0;                       // size in bytes
    mbox[7] = 0x0;                     // end tag
    // tags end
    mbox_call( 8);
    uart_puts("VC Core base addr: 0x");
    uart_hex(mbox[5]);
    uart_puts(", size: 0x");
    uart_hex(mbox[6]);
    uart_puts("\n");
}