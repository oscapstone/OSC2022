#include "uart.h"

#define MMIO_BASE                       0x3F000000

#define MBOX_BASE                       (MMIO_BASE + 0xb880)

// address map
#define MBOX_READ                       (unsigned int*)(MBOX_BASE)
#define MBOX_STATUS                     (unsigned int*)(MBOX_BASE + 0x18)
#define MBOX_WRITE                      (unsigned int*)(MBOX_BASE + 0x20)

// flag
#define MBOX_EMPTY                      0x40000000
#define MBOX_FULL                       0x80000000

// code
#define MBOX_CODE_BUF_REQ               0x00000000
#define MBOX_CODE_BUF_RES_SUCC          0x80000000
#define MBOX_CODE_TAG_REQ               0x00000000

// tag
#define MBOX_TAG_GET_BOARD_REVISION     0x00010002
#define MBOX_TAG_GET_VC_MEMORY          0x00010006
#define MBOX_TAG_SET_CLOCK_RATE         0x00038002
#define MBOX_TAG_SET_PHY_WIDTH_HEIGHT   0x00048003
#define MBOX_TAG_SET_VTL_WIDTH_HEIGHT   0x00048004
#define MBOX_TAG_SET_VTL_OFFSET         0x00048009
#define MBOX_TAG_SET_DEPTH              0x00048005
#define MBOX_TAG_SET_PIXEL_ORDER        0x00048006
#define MBOX_TAG_ALLOCATE_BUFFER        0x00040001
#define MBOX_TAG_GET_PITCH              0x00040008


int mbox_call(unsigned int* mbox, unsigned char channel) {
    unsigned int r = (unsigned int)(((unsigned long)mbox) & (~0xF)) | (channel & 0xF);
    // wait until full flag unset
    while (*MBOX_STATUS & MBOX_FULL) {
    }
    // write address of message + channel to mailbox
    *MBOX_WRITE = r;
    // wait until response
    while (1) {
        // wait until empty flag unset
        while (*MBOX_STATUS & MBOX_EMPTY) {
        }
        // is it a response to our msg?
        if (r == *MBOX_READ) {
            // check is response success
            return mbox[1] == MBOX_CODE_BUF_RES_SUCC;
        }
    }
    return 0;
}

void mbox_board_revision() {
    unsigned int __attribute__((aligned(16))) mbox[7];
    mbox[0] = 7 * 4;  // buffer size in bytes
    mbox[1] = MBOX_CODE_BUF_REQ;
    // tags begin
    mbox[2] = MBOX_TAG_GET_BOARD_REVISION;  // tag identifier
    mbox[3] = 4;                            // maximum of request and response value buffer's length.
    mbox[4] = MBOX_CODE_TAG_REQ;            // tag code
    mbox[5] = 0;                            // value buffer
    mbox[6] = 0x0;                          // end tag
    // tags end
    mbox_call(mbox, 8);
    uart_printf("Board Revision: %x\n", mbox[5]);
}

void mbox_vc_memory() {
    unsigned int __attribute__((aligned(16))) mbox[8];
    mbox[0] = 8 * 4;  // buffer size in bytes
    mbox[1] = MBOX_CODE_BUF_REQ;
    // tags begin
    mbox[2] = MBOX_TAG_GET_VC_MEMORY;  // tag identifier
    mbox[3] = 8;                       // maximum of request and response value buffer's length.
    mbox[4] = MBOX_CODE_TAG_REQ;       // tag code
    mbox[5] = 0;                       // base address
    mbox[6] = 0;                       // size in bytes
    mbox[7] = 0x0;                     // end tag
    // tags end
    mbox_call(mbox, 8);
    uart_printf("VC Core base addr: 0x%x size: 0x%x\n", mbox[5], mbox[6]);
}