#ifndef VFS_H
#define VFS_H

#include "stddef.h"
#include "stdint.h"
#include "list.h"

#define VFS_RO 10
#define VFS_RW 11

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

enum vtype { VDIR, VFILE };

struct vfs;
struct vnode;

struct vfs {
  struct vfs *vfs_next;
  struct vnode *vfs_vnodecovered;
  void* vfs_data;
};

struct vnode {
  struct vfs *v_mountedvfs;
  struct vfs *v_parent_vfs;
  struct vnodeops *v_ops;
  enum vtype v_type;
  void* v_data;
  struct list *v_dir_children;
};

struct uio {
  char *buf;
  size_t len;
  off_t off;
  size_t ret;
};

struct vnodeops {
  int (*vn_mkdir)(struct vnode* vn_dir, const char *name, struct vnode** target);
  int (*vn_create)(struct vnode* vn_dir, const char *name, struct vnode** target);
  int (*vn_lookup)(struct vnode* vn_dir, const char *name, struct vnode** target);
  int (*vn_remove)(struct vnode* vn_dir, const char *name);
  int (*vn_write)(struct vnode* vn_file, struct uio *uiop);
  int (*vn_read)(struct vnode* vn_file, struct uio *uiop);
  int (*vn_readdir)(struct vnode* vn_dir);
};

struct vfs_file {
  struct vnode *vn;
  off_t pos;
  char *fullpath;
};

int vfs_open(const char *fullpath, struct vfs_file **target, int flag);
int vfs_close(struct vfs_file *file);
int vfs_lseek(struct vfs_file *file);
int vfs_mkdir(const char *fullpath);
int vfs_remove(const char *fullpath);
int vfs_mount(const char *mount_path, struct vfs *fs);
int vfs_lookup(const char *fullpath, struct vnode **target);

int register_filesystem(struct vfs* fs);

extern struct vfs *rootfs;


#endif 
