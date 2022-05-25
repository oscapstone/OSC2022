#include "start.h"

int main() {
  unsigned int  __attribute__((aligned(16))) mbox[8];

    // get serail number
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;
    print_s("\n");
    if (mbox_call(MBOX_CH_PROP,mbox)) {
        print_s("serial number is: ");
        print_h(mbox[6]);
        print_h(mbox[5]);
        print_s("\n");
    }
    // get board revision
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETBDVS;     // get board revision
    mbox[3] = 4;                    // buffer size
    mbox[4] = 4;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;
    if (mbox_call(MBOX_CH_PROP,mbox)) {
        print_s("board revision is: ");
        print_h(mbox[6]);
        print_h(mbox[5]);
        print_s("\n");
    }

    // get arm memory
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETARMMEM;   // get arm memory info
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;
    if (mbox_call(MBOX_CH_PROP,mbox)) {
        print_s("arm base addr: ");
        print_h(mbox[5]);
        print_s("\n");
        print_s("arm addr size: ");
        print_h(mbox[6]);
        print_s("\n");
    }
  // while(1);
  return 0;
}