#include <mailbox.h>

#define GET_BOARD_REVISION_TAG  0x00010002
#define GET_ARM_MEMORY_TAG      0x00010005
#define REQUEST_CODE            0x00000000
#define REQUEST_SUCCEED         0x80000000
#define REQUEST_FAILED          0x80000001
#define TAG_REQUEST_CODE        0x00000000
#define END_TAG                 0x00000000

void get_board_revision(unsigned int* revision){
    unsigned int mailbox[7];
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION_TAG; // tag identifier
    mailbox[3] = 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;

    mailbox_request(8, mailbox);
    *revision = mailbox[5];
    return ;
}

void get_ARM_memory_info(void **addr_base, unsigned int *size){
    unsigned int mailbox[8];
    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_ARM_MEMORY_TAG; // tag identifier
    mailbox[3] = 8; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    mailbox[6] = 0; // value buffer
    // tags end
    mailbox[7] = END_TAG;

    mailbox_request(8, mailbox);
    *addr_base = (void *)(unsigned long long)mailbox[5];
    *size = mailbox[6];
    return ;
}