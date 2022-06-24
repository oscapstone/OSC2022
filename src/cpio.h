
#ifndef __CPIO_H__
#define __CPIO_H__

#include "string.h"
#include "uart.h"
#include "stddef.h"
#include "stdlib.h"

// #define SIM

// #ifdef SIM
// #define CPIO_BASE 0x8000000
// #else
// #define CPIO_BASE 0x20000000
// #endif

typedef struct cpio_newc_header
{
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

} cpio_newc_header_t;

typedef struct cpio_file_pointer
{
    cpio_newc_header_t *header;
    char *filename;
    char *data;
	
} cpio_fp_t;

char *CPIO_BASE;
char *CPIO_END;

void cpio_init(char* name, char *base_addr);
void cpio_cat(char *filename);
void cpio_ls();
void cpio_read_file(void **addr, cpio_fp_t *fp);
int ascii2int(char *str, int len);
void cpio_get_file_info(char *filename, cpio_fp_t *fp);
void copy_prog_from_cpio(char *dst_addr, const char *src_addr, size_t prog_size);

#endif
