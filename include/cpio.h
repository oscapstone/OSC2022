#ifndef CPIO
#define CPIO

void extract_cpio(char* address, int ls, int cat, char* name);
char* load_user_program(char* address, char* program_address, char* name);

#endif