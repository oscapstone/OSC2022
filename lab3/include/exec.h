#ifndef EXEC_H
#define EXEC_H

#define USTACK_SIZE  0x10000
// exec a binary in memory EL1->EL0
int exec(char* data);

#endif