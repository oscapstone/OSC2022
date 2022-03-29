#include "../include/mailbox.h"
#include "../include/UART.h"

#define physical_add 	 0x3F000000
#define MAIL_BASE		 (physical_add + 0xB880)

#define MAILBOX_READ       ((volatile unsigned int*)(MAIL_BASE+0x0))
#define MAILBOX_WRITE      ((volatile unsigned int*)(MAIL_BASE+0x20))
#define MAILBOX_STATUS     ((volatile unsigned int*)(MAIL_BASE+0x18))

#define MAILBOX_RESPONSE   0x80000000
#define MAILBOX_FULL       0x80000000
#define MAILBOX_EMPTY      0x40000000

#define MAILBOX_TAG_GETBOARDREVISION 		0x10002
#define MAILBOX_TAG_MEMADDSIZE		0x10006

unsigned int __attribute__((aligned(16))) mailbox[36]; 

int Msg_passing(unsigned char channel){
	unsigned int r = (((unsigned int)((unsigned long)&mailbox) & ~0X0F));  // reserve the last 4 bits
	r |= (channel&0X0F);  // place the channel number to the last 4 bits

	do{
		asm volatile("nop");
	} while(*MAILBOX_STATUS & MAILBOX_FULL);   // if Mailbox 0 status register’s full flag is set, then wait

	*MAILBOX_WRITE = r;    // write to Mailbox 1 rw register

	while(1){
		do{
			asm volatile("nop");
		} while(*MAILBOX_STATUS & MAILBOX_EMPTY);   // if Mailbox 0 status register’s empty flag is set, then wait

		if(r == *MAILBOX_READ)		// the reply from request
			return mailbox[1] == MAILBOX_RESPONSE;
	}
	return 0;
}

void Board_revision(){
	UART_init();

	mailbox[0] = 7*4;	// buffer size
	mailbox[1] = 0;		// request code
	mailbox[2] = MAILBOX_TAG_GETBOARDREVISION;		// tag ID
	mailbox[3] = 4;		// request and response length
	mailbox[4] = 0;		// request tag
	mailbox[5] = 0;		// value buffer
	mailbox[6] = 0;		// end tag

	Msg_passing(8);    //requests from ARM
}

void Mem_add_size(){
	UART_init();

	mailbox[0] = 8*4;	// buffer size
	mailbox[1] = 0;		// request code
	mailbox[2] = MAILBOX_TAG_MEMADDSIZE;		// tag ID
	mailbox[3] = 8;		// request and response length
	mailbox[4] = 0;		// request tag
	mailbox[5] = 0;		// value buffer
	mailbox[6] = 0;		// value buffer
	mailbox[7] = 0;		// end tag

	Msg_passing(8);   //requests from ARM
}