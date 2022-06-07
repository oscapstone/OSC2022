#ifndef TMPFS_H
#define TMPFS_H

#include "vfs.h"
#include "list.h"

enum tmpfsEntryType { File, Dir };

struct tmpfsFile {
  char *name;
  size_t size;
  char *content;
};

struct tmpfsDir {
  char *dirname;
  struct tmpfsItem *parent;
  struct list children;
};

struct tmpfsItem {
  enum tmpfsEntryType type;
  union {
    struct tmpfsFile *file;
    struct tmpfsDir *dir;
  } entry;
};


// int setup_tmpfs_mount(struct filesystem *fs, struct mount *mount);

struct tmpfsItem* tmpfsCreateFile(struct tmpfsItem *item, const char *filename);
struct tmpfsItem* tmpfsMakeDir(struct tmpfsItem *item, const char *dirname);
size_t tmpfsWrite(struct tmpfsFile *file, const char *buf, size_t size,
               uint32_t offset);
size_t tmpfsRead(struct tmpfsFile *file, char *buf, size_t size,
                 uint32_t offset);
int tmpfsRmDir(struct tmpfsItem *dir_item);
int tmpfsRmFile(struct tmpfsItem *dir_item, struct tmpfsItem *file_item);

void tmpfs_vn_freelist(struct list *lst);
int tmpfs_vn_readdir(struct vnode *vn_dir);

int tmpfs_vn_mkdir(struct vnode* vn_dir, const char *name, struct vnode** target);
int tmpfs_vn_create(struct vnode* vn_dir, const char *name, struct vnode** target);
int tmpfs_vn_lookup(struct vnode* vn_dir, const char *name, struct vnode** target);
int tmpfs_vn_remove(struct vnode* vn_dir, const char *name);
int tmpfs_vn_write(struct vnode* vn_file, struct uio *uiop);
int tmpfs_vn_read(struct vnode *vn_file, struct uio *uiop);

int tmpfs_createfs(struct vfs **target);

extern struct vnodeops tmpfsVNodeOps;

// int tmpfs_vfs_write(struct file *file, const void *buf, size_t len);
// int tmpfs_vfs_read(struct file *file, void *buf, size_t len);
// int tmpfs_vfs_open(struct vnode *filenode, struct file **target);
// int tmpfs_vfs_close(struct file *file);
// long tmpfs_vfs_lseek64(struct file *file, long offset, int whence);


// int *tmpfs_vfs_lookup(struct vnode* dir_node, struct vnode** target,
//                       const char* component_name);
// int *tmpfs_vfs_create(struct vnode* dir_node, struct vnode** target,
//                       const char* component_name);
// int *tmpfs_vfs_mkdir(struct vnode* dir_node, struct vnode** target,
//                      const char* component_name);

// extern struct file_operations tmpfs_file_ops;
// extern struct vnode_operations tmpfs_vnode_ops;

#endif
