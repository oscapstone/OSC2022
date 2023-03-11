#ifndef VFS_H
#define VFS_H

#include <stddef.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define UIO_READ 0
#define UIO_WRITE 1

#define VFS_DELIM "/"

#define EBADVN 150
#define EBADIN 151
// bad internal data

struct vnodeops;
struct vnode;
struct vfsops;

enum vtype { VREG, VDIR };

struct vfs {
  char vfs_name[16];
  struct vfs *vfs_next; // next vfs in list
  struct vfsops *vfs_op; // opeartions on vfs
  struct vnode *vfs_vncovered; // vnode the fs mounted on, not the root
  void* vfs_data; // privated data
};

struct vnode {
  enum vtype vn_type; // vnode type
  int vn_usecount; // reference count of users
  struct vnodeops *vn_ops; // vnode operations
  struct vfs *vn_vfs; // ptr to vfs we are in
  struct vfs *vn_mountedhere; // ptr to fs mounted here(for VDIR)
  void *vn_data; // private data for fs
};

struct uio {
  char *buf;
  off_t off;
  size_t len;
  size_t ret;
};

struct dirent {
  char *name;
  struct vnode* vn;
};

struct vnodeops {
  int (*vn_rdwr)(struct vnode* vn, struct uio *uiop, int rw); // perform read or write on a vnode
  int (*vn_lookup)(struct vnode* vn, const char *name, struct vnode** dst); // lookup a component name name in directory vn.
  int (*vn_create)(struct vnode* vn, const char *name, struct vnode** dst); // create a new file "name" in directory vn, dst points to a pointer to a vnode for the result.
  int (*vn_mkdir)(struct vnode* vn, const char *name, struct vnode** dst); // create directory "name" in directory vn , dst points to a pointer to a vnode for the result.
  int (*vn_readdir)(struct vnode* vn, struct uio *uiop); // read entries from directory vn
};

struct vfsops {
  int (*vfs_mount)(struct vfs* vfsp, const char *path);
  int (*vfs_root)(struct vfs* vfsp, struct vnode** dst);
};

struct file {
  struct vnode* vn;
  off_t pos;
};

extern struct vfs *root;
int init_rootfs();
int vfs_register(struct vfs *fs);

int vfs_open(const char *fullpath, struct file **dst, int creat);
int vfs_close(struct file *file);
int vfs_read(struct file *file, void* buf, size_t len, size_t *read_len);
int vfs_write(struct file *file, const void *buf, size_t len, size_t *written_len);
int vfs_mkdir(const char *fullpath);
int vfs_mount(const char *fullpath, const char *fsname);
int vfs_lookup(const char *fullpath, struct vnode **dst);
int vfs_create(const char *fullpath, struct file **dst);


#define O_CREAT 00000100
int syscall_open(const char *pathname, int flags);
int syscall_close(int fd);
long syscall_write(int fd, const void *buf, unsigned long count);
long syscall_read(int fd, void *buf, unsigned long count);
int syscall_mkdir(const char *pathname, unsigned mode);
int syscall_mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
int syscall_chdir(const char *path);

#endif 
