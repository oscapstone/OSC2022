#include "mailbox.h"
#include "stdint.h"
#include "printf.h"

int mbox_call(unsigned char ch, unsigned int *mbox, unsigned int *mbox_va){
  unsigned int r = (((uint32_t)((uint64_t)mbox)&(~0xF)) | (ch&0xf));
  do{
    asm volatile("nop");
  }while(*MAILBOX_STATUS & MAILBOX_FULL);     // wait until we can write to the mailbox
  *MAILBOX_WRITE = r;                         // write the address of our message to the mailbox with channel identifier
  while(1) {
    do{
      asm volatile("nop");
    }while(*MAILBOX_STATUS & MAILBOX_EMPTY);
    if(r == *MAILBOX_READ){
      return mbox_va[1]==MAILBOX_RESPONSE;    // user ttbr1 to read the value
    }
  }
  return 0;
}
