#ifndef __MBOX__H__
#define __MBOX__H__
#include "mmio.h"
extern volatile unsigned int mbox[36];
#define VIDEOCORE_MBOX_BASE  (MMIO_BASE + 0x0000B880)
#define MBOX_READ            (VIDEOCORE_MBOX_BASE + 0x00)
#define MBOX_POLL            (VIDEOCORE_MBOX_BASE + 0x10)
#define MBOX_SENDER          (VIDEOCORE_MBOX_BASE + 0x14)
#define MBOX_STATUS          (VIDEOCORE_MBOX_BASE + 0x18)
#define MBOX_CONFIG          (VIDEOCORE_MBOX_BASE + 0x1C)
#define MBOX_WRITE           (VIDEOCORE_MBOX_BASE + 0x20)
#define MBOX_RESPONSE        (0x80000000)
#define MBOX_FULL            (0x80000000)
#define MBOX_EMPTY           (0x40000000)

#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

int mbox_call(unsigned char ch);
#endif
