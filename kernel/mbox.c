#include "mbox.h"

unsigned int get_board_revision(volatile unsigned int mbox[36]){
    mbox[0] = 7 * 4; // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_REVISION; // tag identifier
    mbox[3] = 4; // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0; // board revision
    // tags end
    mbox[6] = END_TAG;

    return mailbox_call(mbox, MAILBOX_CH_PROP); 
}

unsigned int get_arm_memory(volatile unsigned int mbox[36]){
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
    
    return mailbox_call(mbox, MAILBOX_CH_PROP);
}

// Make a mailbox call. Returns 0 on failure, non-zero on success
unsigned int mailbox_call(volatile unsigned int mbox[36], unsigned char ch){
    /* Combine the message address (upper 28 bits) with channel number (lower 4 bits) */
    unsigned int req = (((unsigned int)((unsigned long)mbox) & (~0xF)) | (ch & 0xF));
    /* wait until we can write to the mailbox */
    while(*MAILBOX_STATUS & MAILBOX_FULL){asm volatile("nop");}
    /* write the address of our message to the mailbox with channel identifier */
    *MAILBOX_WRITE = req;

    /* now wait for the response */
    while(1){
        /* wait the response signal */
        while(*MAILBOX_STATUS & MAILBOX_EMPTY){asm volatile("nop");}
        /* read the response to compare our req and request_code */
        if(req == *MAILBOX_READ)
            return mbox[1] == MAILBOX_RESPONSE;
    }
    return 0;
}