#ifndef _CPIO_H_
#define _CPIO_H_

#include "utils.h"
#include "types.h"

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

struct fentry {
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
    char *filename;
    uint8_t data;
    struct list_head list;
};
#endif
