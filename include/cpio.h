#ifndef CPIO_H
#define CPIO_H
#include "simple_alloc.h"

// #define INITRD_ADDR (0x8000000)  // QEMU: 0x8000000, Rpi3: 0x20000000
extern void* INITRD_ADDR;

typedef struct cpio_newc_header {
    char c_magic[6];
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];
} cpio_newc_header;

// Compiler 會對 cpio_newc_parse_header 做 alignmenrt..
#define CPIO_NEWC_HEADER_SIZE (0x70 - 2)

typedef void (*cpio_callback_t)(char* param, cpio_newc_header* header, char* file_name, unsigned int name_size, char* file_data, unsigned int data_size);

void cpio_init();
void cpio_newc_parser(cpio_callback_t callback, char* param);
void cpio_newc_parse_header(char** cpio_ptr, cpio_newc_header** header);
void cpio_newc_parse_data(char** cpio_ptr, char** buf, unsigned int size, unsigned int offset);
void cpio_ls_callback(char* param, cpio_newc_header* header, char* file_name, unsigned int name_size, char* file_data, unsigned int data_size);
void cpio_cat_callback(char* param, cpio_newc_header* header, char* file_name, unsigned int name_size, char* file_data, unsigned int data_size);
void cpio_prog_callback(char* param, cpio_newc_header* header, char* file_name, unsigned int name_size, char* file_data, unsigned int data_size);

#endif