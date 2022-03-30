#include "gpio.h"
#include "uart.h"

volatile unsigned int  __attribute__((aligned(16))) mbox[8];

#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MAILBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MAILBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MAILBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MAILBOX_RESPONSE   0x80000000
#define MAILBOX_FULL       0x80000000
#define MAILBOX_EMPTY      0x40000000


int mbox_call(unsigned char ch)
{
  unsigned int r = (((unsigned int)((unsigned long)&mbox)&(~0xF)) | (ch&0xf));
  do{
    asm volatile("nop");
  }while(*MAILBOX_STATUS & MAILBOX_FULL); // wait until we can write to the mailbox

  *MAILBOX_WRITE = r; // write the address of our message to the mailbox with channel identifier

  while(1) {
    do{
      asm volatile("nop");
    }while(*MAILBOX_STATUS & MAILBOX_EMPTY);
    if(r == *MAILBOX_READ)
      return mbox[1]==MAILBOX_RESPONSE;
  }
  return 0;
}
