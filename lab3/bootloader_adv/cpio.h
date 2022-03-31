#ifndef CPIO_H
#define CPIO_H

#define cpio_size 110
#define cpio_addr ((char *)0x20000000)

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

void cpio_ls();
void cpio_cat(char *filename);
int extract_header(cpio_newc_header *cpio_header, char *target);
int align(int alignment, int size);

#endif