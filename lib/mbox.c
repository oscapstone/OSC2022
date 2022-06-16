#include "address.h"
#include "mbox.h"
#include "mmu.h"

volatile unsigned int  __attribute__((aligned(16))) mbox[36];

int _mbox_call(unsigned char ch) {
    unsigned long r = (((unsigned long)((unsigned long)&mbox) & ~0xF) | (ch & 0xF));
    // wait for ready 
    do {
        asm volatile("nop");
    } while(*MBOX_STATUS & MBOX_FULL);

    
    *MBOX_WRITE = r; // write the address of message to the mailbox with channel identifier
    
    while(1) {
        // wait for respose
        do {
            asm volatile("nop");
        } while(*MBOX_STATUS & MBOX_EMPTY);
        
        // make sure it is a response to our message
        if(r == PHY_TO_VIR(*MBOX_READ)) {
            // is it a valid successful response
            return mbox[1] == MBOX_RESPONSE;
        }
    }
    return 0;
}

void mbox_request(unsigned int tag, unsigned int res_len) {
    // get the board's unique serial number with a mailbox call

    int len = (res_len > 4) ? 8 : 7;

    mbox[0] = len * 4;              // length of the message in bytes
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = tag; 
    mbox[3] = res_len;              // buffer size
    mbox[4] = MBOX_TAG_REQUEST;     // 0x00000000
    mbox[5] = 0;                    // clear output buffer

    if(res_len > 4) {
        mbox[6] = 0;
        mbox[7] = MBOX_TAG_LAST;
    }
    else {
        mbox[6] = MBOX_TAG_LAST;
    }
}