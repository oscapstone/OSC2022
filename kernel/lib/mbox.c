/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "mbox.h"
#include "gpio.h"
#include "my_string.h"
#include "uart.h"

void get_board_revision(volatile unsigned int mbox[36]){
    mbox[0] = 7 * 4; // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_REVISION; // tag identifier
    mbox[3] = 4; // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0; // board revision
    // tags end
    mbox[6] = END_TAG;

    mailbox_call(mbox, MAILBOX_CH_PROP); 

    char buf[MAX_LEN];
    unsign_itohexa(mbox[5], buf);
    uart_printf("Board revision: 0x%s\n", buf);
}

void get_arm_memory(volatile unsigned int mbox[36]){
    mbox[0] = 8 * 4; // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = GET_ARM_MEMORY; // tag identifier
    mbox[3] = 8; // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0; // base address in bytes
    mbox[6] = 0; // size in bytes
    // tags end
    mbox[7] = END_TAG;

    mailbox_call(mbox, MAILBOX_CH_PROP);
    char buf[MAX_LEN];
    unsign_itohexa(mbox[5], buf);
    uart_printf("ARM memory base address: 0x%s\n", buf);
    unsign_itohexa(mbox[6], buf);
    uart_printf("ARM memobry size: 0x%s\n", buf);
}

/*
1. Combine the message address (upper 28 bits) with channel number (lower 4 bits)
2. Check if Mailbox 0 status register’s full flag is set.
3. If not, then you can write to Mailbox 1 Read/Write register.
4. Check if Mailbox 0 status register’s empty flag is set.
5. If not, then you can read from Mailbox 0 Read/Write register.
6. Check if the value is the same as you wrote in step 1.
*/
unsigned int mailbox_call(volatile unsigned int mbox[36], unsigned char ch){
  /* Combine the message address (upper 28 bits) with channel number (lower 4 bits) */
  unsigned int req = (((unsigned int)((unsigned long)mbox) & (~0xF)) | (ch & 0xF));
  /* wait until we can write to the mailbox */
  while(*MAILBOX_STATUS & MAILBOX_FULL){asm volatile("nop");}
  *MAILBOX_WRITE = req;

  /* now wait for the response */
  while(1){
    /* wait the response signal */
    while(*MAILBOX_STATUS & MAILBOX_EMPTY){asm volatile("nop");}

    /* read the response to compare the our req and request_code */
    if(req == *MAILBOX_READ){
      return mbox[1] == MAILBOX_RESPONSE;
    }
  }
  return 0;
}

