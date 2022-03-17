#include "mailbox.h"

#define MAILBOX_BASE    0x3f00b880

#define MAILBOX_READ    ((volatile unsigned int*)MAILBOX_BASE)
#define MAILBOX_STATUS  ((volatile unsigned int*)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE   ((volatile unsigned int*)(MAILBOX_BASE + 0x20))

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000


#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY      0x00010005
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

int mailbox_call(unsigned int *msg, unsigned char channel) {
  unsigned long r = ((unsigned long)msg & 0xFFFFFFF0) | (channel & 0xF);
  while ((*MAILBOX_STATUS) & MAILBOX_FULL);
  *MAILBOX_WRITE = r;
  while (1) {
    while ((*MAILBOX_STATUS) & MAILBOX_EMPTY);
    if (*MAILBOX_READ == r) {
      return msg[1] == REQUEST_SUCCEED;
    }
  }
}

unsigned int get_board_revision(){
  unsigned int __attribute__((aligned(16))) mailbox[7];
  mailbox[0] = 7 * 4; // buffer size in bytes
  mailbox[1] = REQUEST_CODE;
  // tags begin
  mailbox[2] = GET_BOARD_REVISION; // tag identifier
  mailbox[3] = 4; // maximum of request and response value buffer's length.
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0; // value buffer
  // tags end
  mailbox[6] = END_TAG;

  mailbox_call(mailbox, 8); // message passing procedure call, you should implement it following the 6 steps provided above.
  return mailbox[5];
}

void get_arm_memory(unsigned int base_and_size[2]){
  unsigned int __attribute__((aligned(16))) mailbox[8];
  mailbox[0] = 8 * 4; // buffer size in bytes
  mailbox[1] = REQUEST_CODE;
  // tags begin
  mailbox[2] = GET_ARM_MEMORY; // tag identifier
  mailbox[3] = 8; // maximum of request and response value buffer's length.
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0; // value buffer
  mailbox[6] = 0;
  // tags end
  mailbox[7] = END_TAG;

  mailbox_call(mailbox, 8); // message passing procedure call, you should implement it following the 6 steps provided above.
  base_and_size[0] = mailbox[5];
  base_and_size[1] = mailbox[6];
}

