#include "mailbox.h"
#include "printf.h"

/* mailbox message buffer */
volatile unsigned int mbox_buffer[64] ALIGN(16);

int mailbox_call(unsigned char channel)
{
	unsigned int address = (unsigned int)(((unsigned long)mbox_buffer & ~0xF) | (channel & 0xF));

 	//printf("mailbox_call check prewrite\r\n");
	while (*MAILBOX1_STATUS & ARM_MS_FULL);

 	*MAILBOX1_WRITE = address;

 	while (1) {
		//printf("mailbox_call check preread\r\n");
		while (*MAILBOX0_STATUS & ARM_MS_EMPTY);

 		//printf("mailbox_call reading\r\n");
		if (address == *MAILBOX0_READ)
			return mbox_buffer[1] == MBOX_RESPONSE;
	}
	printf("mailbox_call error\r\n");
	return 0;
}

 void get_serial(void)
{
	mbox_buffer[0] = 8 * 4;
	mbox_buffer[1] = MBOX_REQUEST;
	mbox_buffer[2] = MBOX_TAG_GETSERIAL; // tag identifier
	mbox_buffer[3] = 8; // buffer size
	mbox_buffer[4] = 8; // buffer size
	mbox_buffer[5] = 0; // clear buffer data
	mbox_buffer[6] = 0; // clear buffer data
	mailbox_call(MBOX_CH_PROP);
	printf("My serial number : 0x%x  0x%x\r\n", mbox_buffer[6], mbox_buffer[5]);
}

void get_board_revision(){
	mbox_buffer[0] = 7 * 4; // buffer size in bytes
	mbox_buffer[1] = REQUEST_CODE;
	// tags begin
	mbox_buffer[2] = GET_BOARD_REVISION; // tag identifier
	mbox_buffer[3] = 4; // maximum of request and response value buffer's length.
	mbox_buffer[4] = TAG_REQUEST_CODE;
	mbox_buffer[5] = 0; // value buffer
	// tags end
	mbox_buffer[6] = END_TAG;

	mailbox_call(MBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.
	printf("Board revision : 0x%x\r\n", mbox_buffer[5]); // it should be 0xa020d3 for rpi3 b+
}

void get_arm_memory_info(){
	mbox_buffer[0] = 8 * 4; // buffer size in bytes
	mbox_buffer[1] = REQUEST_CODE;
	// tags begin
	mbox_buffer[2] = GET_ARM_MEMORY; // tag identifier
	mbox_buffer[3] = 8; // maximum of request and response value buffer's length.
	mbox_buffer[4] = TAG_REQUEST_CODE;
	mbox_buffer[5] = 0; // value buffer
	// tags end
	mbox_buffer[6] = END_TAG;

	mailbox_call(MBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.
	printf("ARM memory base address : 0x%x\r\n", mbox_buffer[5]);
	printf("ARM memory size : 0x%x\r\n", mbox_buffer[6]);
}
