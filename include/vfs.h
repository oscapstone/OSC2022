#ifndef H_VFS
#define H_VFS

#include "type.h"

#define MAX_COMPONENT_LENGTH 15
#define MAX_PATHNAME 257
#define MAX_ENTRY 16
#define MAX_FILE_SIZE 4096
#define MAX_FD 16
#define MAX_REG_FS 16
#define MAX_REG_DEV 16

#define O_CREAT 00000100
#define SEEK_SET 0

enum NodeType { nt_dir, nt_file };

typedef struct vnode {
  struct mount* mount;
  struct vnode_operations* v_ops;
  struct file_operations* f_ops;
  void* internal;
} vnode_t;

// file handle
typedef struct file {
  struct vnode* vnode;
  size_t f_pos;  // RW position of this file handle
  struct file_operations* f_ops;
  int flags;
} file_t;

typedef struct mount {
  struct vnode* root;
  struct filesystem* fs;
} mount_t;

typedef struct filesystem {
  const char* name;
  int (*setup_mount)(struct filesystem* fs, struct mount* mount);
} filesystem_t;

typedef struct file_operations {
  int (*write)(struct file* file, const void* buf, size_t len);
  int (*read)(struct file* file, void* buf, size_t len);
  int (*open)(struct vnode* file_node, struct file** target);
  int (*close)(struct file* file);
  long (*lseek64)(struct file* file, long offset, int whence);
  int (*getsize)(struct vnode* target);
} file_operations_t;

typedef struct vnode_operations {
  int (*lookup)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*create)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*mkdir)(struct vnode* dir_node, struct vnode** target,
              const char* component_name);
} vnode_operations_t;

struct mount* rootfs;
filesystem_t registered_fs[MAX_REG_FS];
file_operations_t registered_dev_fops[MAX_REG_FS];

int register_filesystem(struct filesystem* fs);
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, size_t len);
int vfs_read(struct file* file, void* buf, size_t len);
long vfs_lseek64(struct file* file, long offset, int whence);
int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);
int vfs_lookup(const char* pathname, struct vnode** target);
int nop_op();

int register_dev_fop(file_operations_t *fop);
int vfs_mknod(const char *pathname, int dev_id);
void init_vfs();

#endif