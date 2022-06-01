#ifndef _VFS_HEADER_
#define _VFS_HEADER_
#include "utils.h"
#define O_CREAT 00000100

enum retMsg{
    errMsg = -1,
    sucessMsg = 0,
    compNotFound = 1,
    lastCompNotFound = 2
};

struct vnode {
  struct mount* mount;
  struct vnode_operations* v_ops;
  struct file_operations* f_ops;
  void* internal;
};

// file handle
struct file {
  struct vnode* vnode;
  size_t f_pos;  // RW position of this file handle
  struct file_operations* f_ops;
  int flags;
};

struct mount {
  struct vnode* root;
  struct filesystem* fs;
};

struct filesystem {
  char name[20];
  int (*setup_mount)(struct filesystem* fs, struct mount* mount);
  struct mount* mount_point;
};

struct file_operations {
  int (*write)(struct file* file, const void* buf, size_t len);
  int (*read)(struct file* file, void* buf, size_t len);
  int (*open)(struct vnode* file_node, struct file** target);
  int (*close)(struct file* file);
  long (*lseek64)(struct file* file, long offset, int whence);
};

struct vnode_operations {
  int (*lookup)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*create)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*mkdir)(struct vnode* dir_node, struct vnode** target,
              const char* component_name);
  void (*ls)(struct vnode* dir_node);
};

void rootfs_init(char* name);
int register_filesystem(struct filesystem* fs);
int vfs_open(const char* pathname, int flags, struct file** target);

int vfs_close(int fid);
int vfs_write(int fid, const void* buf, size_t len);

int vfs_read(int fid, void* buf, size_t len);

int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);
int vfs_lookup(const char* pathname, struct vnode** target);
void vfs_ls();
int vfs_chdir(const char* path);
struct vnode* vnode_create(struct vnode*,struct mount*,struct vnode_operations*,struct file_operations*,int);
void testfs_exec();
void test_fs();
#endif