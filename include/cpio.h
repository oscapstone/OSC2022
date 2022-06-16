#ifndef CPIO
#define CPIO

#include "type.h"

#define MAX_INITRAMFS_SIZE 0x100000

unsigned int hex2num(char *s, unsigned int max_len);
void extract_cpio(char* address, int ls, int cat, char* name);
char* extract_section(char* context, char* p, char* address, int len);
char* load_user_program(char* address, char* program_address, char* name);
uint64 get_program_size(char* address, char* name);
#endif