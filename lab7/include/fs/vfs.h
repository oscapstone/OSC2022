#ifndef _VFS_H_
#define _VFS_H_
#include "lib/list.h"
#include "asm.h"
#include "lib/print.h"
#include "types.h"
#include "lib/string.h"
typedef uint64_t umode_t;
typedef uint64_t loff_t;
typedef uint64_t dev_t;
#define NR_OPEN_DEFAULT 64

#define O_CREAT 00000100

#define DENTRY_FLAG_MOUNTED         (1 << 0)

#define 	S_IFMT   00170000
#define 	S_IFSOCK   0140000
#define 	S_IFLNK   0120000
#define 	S_IFREG   0100000
#define 	S_IFBLK   0060000
#define 	S_IFDIR   0040000
#define 	S_IFCHR   0020000
#define 	S_IFIFO   0010000

#define 	S_ISLNK(m)   (((m) & S_IFMT) == S_IFLNK)
#define 	S_ISREG(m)   (((m) & S_IFMT) == S_IFREG)
#define 	S_ISDIR(m)   (((m) & S_IFMT) == S_IFDIR)
#define 	S_ISCHR(m)   (((m) & S_IFMT) == S_IFCHR) 
#define 	S_ISBLK(m)   (((m) & S_IFMT) == S_IFBLK)
#define 	S_ISFIFO(m)   (((m) & S_IFMT) == S_IFIFO)
#define 	S_ISSOCK(m)   (((m) & S_IFMT) == S_IFSOCK)


struct mount{
    struct dentry* mnt_root;
    struct list_head list;
};

struct filesystem_type{
    char* f_name;
    struct mount* (*mount)(struct filesystem_type*, struct dentry*);
    struct list_head list;
};

struct inode{
    umode_t i_modes;
    struct inode_operations	*i_ops;
    struct file_operations  *f_ops;
    void * private_data;
};

struct dentry{
    char* d_name;
    struct dentry* d_parent;
    int d_flags;

    struct inode *d_inode;
};

struct file{
        struct dentry          *f_dentry;     /* associated dentry object */
        struct file_operations *f_ops;         /* file operations table */
        uint64_t               f_count;       /* file object's usage count */
        loff_t 	               f_pos;
        unsigned int           f_flags;       /* flags specified on open */
        umode_t                f_mode;        /* file access mode */
};

struct fs_struct{
    struct dentry *pwd;
    struct dentry *root;
};

struct files_struct{
    struct file* fd_array[NR_OPEN_DEFAULT];
};

struct inode_operations{
	struct dentry *(*create) (struct dentry *, const char*);
	struct dentry * (*lookup) (struct dentry *, char*);
	int (*link) (struct dentry *,struct inode *,struct dentry *);
	int (*mkdir) (struct dentry *, const char *, umode_t);
	int (*mknod) (struct inode *,struct dentry *,umode_t,dev_t);
};

struct file_operations{
	loff_t (*lseek64) (struct file *, loff_t, int);
	long (*read) (struct file *, char *, size_t, loff_t *);
	long (*write) (struct file *, char *, size_t, loff_t *);
	struct file* (*open) (struct dentry *, unsigned int, umode_t);
    int (*flush) (struct file *);
	int (*release) (struct inode *, struct file *);
};

extern struct file* vfs_open(const char*, int, umode_t);
extern int vfs_close(struct file*);
extern long vfs_write(struct file*, char*, ssize_t);
extern long vfs_read(struct file*, char*, ssize_t);
extern struct dentry* vfs_lookup(const char*);
extern int vfs_mount(const char*, const char*);
extern struct dentry* create_dentry(char *, int, struct inode*);
extern struct inode* create_inode(struct file_operations *, struct inode_operations *, umode_t);
extern int register_filesystem(struct filesystem_type*);
extern void vfs_init(struct filesystem_type*);
extern int get_unused_fd(struct files_struct* );
extern int put_unused_fd(struct files_struct*, int);
extern int fd_install(struct files_struct*, int, struct file*);
extern struct file* get_file_by_fd(struct files_struct*, int);
extern int vfs_mkdir(const char*, umode_t);
extern struct file* create_file(struct dentry* , unsigned int, unsigned);

extern struct mount* rootfs;
#endif
