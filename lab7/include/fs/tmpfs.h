#ifndef _TMPFS_H_
#define _TMPFS_H_
#include "fs/vfs.h"
#include "types.h"
#include "mm/mmu.h"

#define MAX_NUM_DIR_ENTRY 20
#define MAX_FILE_SIZE PAGE_SIZE

struct tmpfs_dir{
    uint64_t count;
    struct dentry* entries[MAX_NUM_DIR_ENTRY];
};

struct tmpfs_file{
    size_t size;
    void* data;
};

extern struct filesystem_type tmpfs;
extern struct inode_operations tmpfs_i_ops;
extern struct file_operations tmpfs_f_ops;
extern struct mount* tmpfs_mount(struct filesystem_type*, struct dentry*);
extern struct dentry* tmpfs_create(struct dentry *, const char*);
extern struct dentry *tmpfs_lookup(struct dentry *, char*);
extern loff_t tmpfs_lseek64(struct file *, loff_t, int);
extern long tmpfs_read(struct file *, char *, size_t, loff_t *);
extern long tmpfs_write(struct file *, char *, size_t, loff_t *);
extern struct file* tmpfs_open(struct dentry *,uint32_t , umode_t );
extern int tmpfs_flush(struct file *);
extern int tmpfs_release(struct inode *, struct file*);
extern int tmpfs_mkdir(struct dentry *, const char *, umode_t);
extern struct dentry* create_tmpfs_file(const char*, struct dentry*, umode_t);
#endif
