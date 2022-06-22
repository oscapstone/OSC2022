#ifndef _TMPFS_H_
#define _TMPFS_H_
#include "fs/vfs.h"
#include "types.h"

#define MAX_NUM_DIR_ENTRY 20

struct tmpfs_dir{
    uint64_t count;
    struct dentry* entries[MAX_NUM_DIR_ENTRY];
};

struct tmpfs_file{
    size_t size;
    void* data;
    size_t capacity;
};


extern struct filesystem_type tmpfs;
extern struct inode_operations tmpfs_i_ops;
extern struct file_operations tmpfs_f_ops;
extern struct mount* tmpfs_mount(struct filesystem_type*, struct dentry*);
extern int tmpfs_create(struct inode *,struct dentry *, umode_t);
extern struct dentry *tmpfs_lookup(struct dentry *, char*);
extern loff_t tmpfs_lseek64(struct file *, loff_t, int);
extern ssize_t tmpfs_read(struct file *, char *, size_t, loff_t *);
extern ssize_t tmpfs_write(struct file *, char *, size_t, loff_t *);
extern int tmpfs_open(struct inode *, struct file *);
extern int tmpfs_flush(struct file *);
extern int tmpfs_release(struct inode *, struct file*);
#endif
