#ifndef MBOX
#define MBOX

/* a properly aligned buffer */
extern volatile unsigned int mbox[36];

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
#define MBOX_TAG_REQUEST                0x0000000
#define MBOX_TAG_LAST                   0x0000000
#define MBOX_TAG_GETBOARD_REVISION      0x0010002
#define MBOX_TAG_GETSERIAL              0x0010004
#define MBOX_TAG_GETARM_RAM             0x0010005


int mbox_call(unsigned char ch);
void mbox_request(unsigned int tag, unsigned int res_len);

#endif