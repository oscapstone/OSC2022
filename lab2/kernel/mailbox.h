#ifndef MAILBOX_H
#define MAILBOX_H

int mailbox_call(unsigned int *msg, unsigned char channel);
unsigned int get_board_revision();
void get_arm_memory(unsigned int base_and_size[2]);

#endif
