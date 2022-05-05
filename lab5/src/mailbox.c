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

int syscall_mbox_call(unsigned char ch, unsigned int *mbox) {
  unsigned long r = ((unsigned long)mbox & 0xFFFFFFF0) | (ch & 0xF);
  while ((*MAILBOX_STATUS) & MAILBOX_FULL);
  *MAILBOX_WRITE = r;
  while (1) {
    while ((*MAILBOX_STATUS) & MAILBOX_EMPTY);
    if (*MAILBOX_READ == r) {
      return mbox[1] == REQUEST_SUCCEED;
    }
  }
}
