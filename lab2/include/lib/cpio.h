#ifndef _CPIO_H_
#define _CPIO_H_

#include "utils.h"
#include "types.h"

#define FILE_TYPE_MASK          0170000
#define FILE_TYPE_SOCKET        0140000
#define FILE_TYPE_SYMLINK       0120000  
#define FILE_TYPE_REGULAR       0100000  
#define FILE_TYPE_BLKDEV        0060000  
#define FILE_TYPE_DIR           0040000  
#define FILE_TYPE_CHRDEV        0020000  
#define FILE_TYPE_NAMED_PIPE    0010000  
#define FILE_SUID               0004000  
#define FILE_SGID               0002000  
#define FILE_STICKY             0001000  
#define FILE_PERMISSION_MASK    0000777

struct cpio_iter{
    uint8_t *cur;
};
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
} __attribute((packed));

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
    uint8_t *data;
    struct list_head list;
};
void cpio_iter_parse(struct cpio_iter*, struct fentry*);
extern void cpio_iter_init(struct cpio_iter*, void*);
extern int cpio_is_tailer(struct fentry*);
#endif
