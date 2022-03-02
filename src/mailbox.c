/* 
The mailbox mechanism consists of three components mailbox registers, channels, and messages.
mailbox0: CPU(ARM) read from GPU(VideoCoreIV),
mailbox0 status: check GPU status.
mailbox1: CPU write to GPU.

Mailbox 0 defines the following channels:

0: Power management
1: Framebuffer
2: Virtual UART
3: VCHIQ
4: LEDs
5: Buttons
6: Touch screen
7:
8: Property tags (ARM -> VC)
9: Property tags (VC -> ARM)
*/
#include "mailbox.h"
// mailbox address and flags

//#define MMIO_BASE   0x3F000000
#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MAILBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MAILBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MAILBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MAILBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MAILBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MAILBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_REQUEST    0

#define MAILBOX_RESPONSE   0x80000000
#define MAILBOX_FULL       0x80000000
#define MAILBOX_EMPTY      0x40000000
#define TAG_REQUEST_CODE   0x00000000
#define GET_BOARD_REVISION 0x00010002
#define GET_ARM_MEMORY     0x00010005

#define END_TAG            0x00000000

int mailbox_call(volatile unsigned int* mailbox,unsigned char ch){
    //unsigned char ch = (unsigned char)MBOX_CH_PROP;
    
     unsigned int r = (unsigned int)(((unsigned long)mailbox) & (~0xF)) | (ch & 0xF);
    // wait for the mailbox not full.
    do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_FULL);

    // write the message address(28 bits) and channel number (4bits) to mailbox write register.
    *MAILBOX_WRITE = r;

    while(1) {
        // is there any response in mailbox
        while(*MAILBOX_STATUS & MAILBOX_EMPTY){asm volatile("nop");};
        if(r == *MAILBOX_READ) 
        {
            return mailbox[1]==MAILBOX_RESPONSE; // check if it is the response, if failed: 0x80000001
        }
            

    }
    return 0;

}

//board revision number: identify what model of pi it is.
void get_board_revision(){
  //unsigned int mailbox[7];
    volatile unsigned int __attribute__((aligned(16))) mailbox[7];
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = MBOX_REQUEST;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION; // tag identifier
    mailbox[3] = 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;

    if(mailbox_call(mailbox,MBOX_CH_PROP)) // message passing procedure call, you should implement it following the 6 steps provided above.
    {
        writes_uart("Get Board revision: ");
        writehex_uart(mailbox[5]); // it should be 0xa020d3 for rpi3 b+
        writes_uart("\r\n");
    }
    else{
        writes_uart("Get Board revision failed!\r\n");
    }
    
}

/*Get ARM memory
Tag: 0x00010005
Request:
Length: 0
Response:
Length: 8
Value:
u32: base address in bytes
u32: size in bytes
Future formats may specify multiple base+size combinations.*/

void get_ARM_memory(){
    volatile unsigned int __attribute__((aligned(16))) mailbox[8];
    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = MBOX_REQUEST;
    // tags begin
    mailbox[2] = GET_ARM_MEMORY; // tag identifier
    mailbox[3] = 8; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    mailbox[6] = 0; // value buffer
    // tags end
    mailbox[7] = END_TAG;

    mailbox_call(mailbox,MBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.
    writes_uart("Get ARM Memory address: ");
    writehex_uart(mailbox[5]); 
    writes_uart("\r\n");
    writes_uart("Get ARM Memory size: ");
    writehex_uart(mailbox[6]); 
    writes_uart("\r\n");
}
