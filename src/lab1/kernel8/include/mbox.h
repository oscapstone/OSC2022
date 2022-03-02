// #ifndef	_P_BASE_H
// #define	_P_BASE_H

#define MMIO_BASE 0x3F000000
// #define MAILBOX_BASE    MMIO_BASE + 0xb880

// #endif  /*_P_BASE_H */

// #ifndef _MAILBOX_H
// #define _MAILBOX_H

// #define MAILBOX_READ        ((volatile unsigned int *)(MAILBOX_BASE))
// #define MAILBOX_STATUS      ((volatile unsigned int *)(MAILBOX_BASE + 0x18))
// #define MAILBOX_WRITE       ((volatile unsigned int *)(MAILBOX_BASE + 0x20))

// #define MAILBOX_EMPTY   0x40000000
// #define MAILBOX_FULL    0x80000000
// #define MAILBOX_RESPONSE   0x80000000

// #define GET_BOARD_REVISION  ((volatile unsigned int *)(0x00010002))
// #define GET_ARM_MEMORY      ((volatile unsigned int *)(0x00010005))
// #define REQUEST_CODE        ((volatile unsigned int *)(0x00000000))
// #define REQUEST_SUCCEED     ((volatile unsigned int *)(0x80000000))
// #define REQUEST_FAILED      ((volatile unsigned int *)(0x80000001))
// #define TAG_REQUEST_CODE    ((volatile unsigned int *)(0x00000000))
// #define END_TAG             ((volatile unsigned int *)(0x00000000))

#define MBOX_TAG_GET_VC_MEMORY  0x00010006
#define GET_BOARD_REVISION  0x00010002
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

int mbox_call(unsigned char ch);
void get_board_revision();
void mbox_vc_memory();