#ifndef _MAILBOX_H_
#define _MAILBOX_H_

extern unsigned int mailbox[36];

int Msg_passing(unsigned char channel);
void Board_revision();
void Mem_add_size();

#endif