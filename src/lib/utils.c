#include "utils.h"

void get_board_revision (unsigned int* board_revision) {
    mbox[0] = 7 * 4;              // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_REVISION; // tag identifier
    mbox[3] = 4;                  // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0;                  // value buffer
    // tags end
    mbox[6] = END_TAG;

    mbox_call(MBOX_CH_PROP);

    // Should be 0xA020D3 for rpi3 b+
    *board_revision = mbox[5];
    return;
}

void get_board_serial(unsigned int* msb, unsigned int* lsb) {
    mbox[0] = 8 * 4;           
    mbox[1] = REQUEST_CODE;

    mbox[2] = GET_BOARD_SERIAL;
    mbox[3] = 8;          
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0;
    mbox[6] = 0;

    mbox[7] = END_TAG;
    
    if (mbox_call(MBOX_CH_PROP)) 
    {
        *msb = mbox[6];
        *lsb = mbox[5];
    }
    else 
    {
        *msb = 0xFFFFFFFF;
        *lsb = 0xFFFFFFFF;
    }

    return;
}

void get_memory_info (unsigned int* mem_base, unsigned int* mem_size ) {
    mbox[0] = 8 * 4;           
    mbox[1] = REQUEST_CODE;

    mbox[2] = GET_MEMORY_BASE;
    mbox[3] = 8;
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0;              
    mbox[6] = 0;          

    mbox[7] = END_TAG;

    if (mbox_call(MBOX_CH_PROP)) 
    {
        *mem_size = mbox[6];
        *mem_base = mbox[5];
    } 
    else
    {
        *mem_size = 0xFFFFFFFF;
        *mem_base = 0xFFFFFFFF;
    } 

    return;
}
