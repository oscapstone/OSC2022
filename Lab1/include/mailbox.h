#ifndef	_MAILBOX_H
#define	_MAILBOX_H

#define VIDEOCORE_MBOX (0x3F00B880)
#define MAILBOX0_READ   ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x00))
#define MAILBOX0_PEEK   ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x10))
#define MAILBOX0_SENDER ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x14))
#define MAILBOX0_STATUS ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x18))
#define MAILBOX0_CONFIG ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x1C))
#define MAILBOX1_WRITE  ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x20))
#define MAILBOX1_PEEK   ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x30))
#define MAILBOX1_SENDER ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x34))
#define MAILBOX1_STATUS ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x38))
#define MAILBOX1_CONFIG ((volatile unsigned int*)(VIDEOCORE_MBOX + 0x3C))
#define ARM_MS_FULL           0x80000000
#define MBOX_RESPONSE         0x80000000
#define ARM_MS_EMPTY          0x40000000
#define ARM_MC_IHAVEDATAIRQEN 0x00000000

#define MBOX_REQUEST    0

/* channels */
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

/* tags */
#define MBOX_TAG_GETSERIAL      0x10004
#define MBOX_TAG_LAST           0

#define ALIGN(n) __attribute__((aligned(n)))

#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY      0x00010005
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

int mailbox_call(unsigned char channel);
void get_serial(void);
void get_board_revision(void);
void get_arm_memory_info();

#endif  /*_MAILBOX_H */
