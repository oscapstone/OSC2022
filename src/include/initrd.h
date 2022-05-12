#include <stdint.h>
#include <stddef.h>
#include <kmalloc.h>

#ifndef _DEF_INITRD
#define _DEF_INITRD

// extern char* INITRD_ADDRESS_START;

struct cpio_newc_header {
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
};

typedef struct INITRD_FILE_{
    uint32_t ino;
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t nlink;
    uint32_t mtime;
    uint32_t filesize;
    uint32_t devmajor;
    uint32_t devminor;
    uint32_t rdevmajor;
    uint32_t rdevminor;
    uint32_t namesize;
    uint32_t check;
    char* header;
    char* filename;
    char* filecontent;
    struct INITRD_FILE_* nextfile;
} INITRD_FILE;

INITRD_FILE* initrd_parse();
INITRD_FILE* initrd_list();
INITRD_FILE* initrd_get(const char *filename);
int initrd_init();

#endif