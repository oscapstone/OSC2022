#include "mbox.h"

volatile unsigned int  __attribute__((aligned(16))) mbox[72];

unsigned int get_board_revision(unsigned int* board_revision){
    mbox[0] = 7 * 4; // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_REVISION; // tag identifier
    mbox[3] = 4; // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0; // board revision
    // tags end
    mbox[6] = END_TAG;

    if (mailbox_call(MAILBOX_CH_PROP)) {
        *board_revision = mbox[5];
        return 0;
    }
    else {
        // uart_printf("Unable to query serial\r\n");
        return -1;
    }
}

unsigned int get_arm_memory(unsigned int* base_addr,unsigned int* size){
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

    if (mailbox_call(MAILBOX_CH_PROP)) {
        *base_addr = mbox[5];
        *size = mbox[6];
        return 0;
    }
    else {
        // uart_printf("Unable to query serial\r\n");
        return -1;
    }
}

// Make a mailbox call. Returns 0 on failure, non-zero on success
unsigned int mailbox_call(unsigned char ch){
    /* Combine the message address (upper 28 bits) with channel number (lower 4 bits) */
    unsigned int req = (((unsigned int)((unsigned long)mbox) & (~0xF)) | (ch & 0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MAILBOX_WRITE = req;

    /* now wait for the response */
    while (1) {
        /* wait the response signal */
        do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_EMPTY);
        /* read the response to compare our req and request_code */
        if(req == PHYS_TO_VIRT(*MAILBOX_READ))
            return mbox[1] == MAILBOX_RESPONSE;
    }
    return 0;
}