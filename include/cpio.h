#ifndef CPIO
#define CPIO

#include "type.h"

#define MAX_INITRAMFS_SIZE 0x10000

void extract_cpio(char* address, int ls, int cat, char* name);
char* load_user_program(char* address, char* program_address, char* name);
uint64 get_program_size(char* address, char* name);
#endif