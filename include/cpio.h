#ifndef _CPIO_HEADER_
#define _CPIO_HEADER_
#include "utils.h"
#include "dtb.h"
extern uint32_t* cpio_addr;
typedef struct {
    char c_magic[6];      /* Magic header '070701'. */
    char c_ino[8];        /* "i-node" number. */
    char c_mode[8];       /* Permisions. */
    char c_uid[8];        /* User ID. */
    char c_gid[8];        /* Group ID. */
    char c_nlink[8];      /* Number of hard links. */
    char c_mtime[8];      /* Modification time. */
    char c_filesize[8];   /* File size. */
    char c_devmajor[8];   /* Major dev number. */
    char c_devminor[8];   /* Minor dev number. */
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];   /* Length of filename in bytes. */
    char c_check[8];      /* Checksum. */
}cpio_newc_header;
void cpio_ls();
void cpio_cat();
unsigned long cpio_get_addr(char** filedata,unsigned long* filesize);
//void* initramfs_callback(fdt_prop* prop,char * name,uint32_t len_prop);
void* initramfs_start_callback(fdt_prop* prop,char * name,uint32_t len_prop);
void* initramfs_end_callback(fdt_prop* prop,char * name,uint32_t len_prop);
unsigned long parse_hex_str(char *s, unsigned int max_len);
int cpio_parse_header(cpio_newc_header *archive,
        char **filename, unsigned long *_filesize, char **data,
        cpio_newc_header **next);
#endif