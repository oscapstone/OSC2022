#ifndef TMPFS_H
#define TMPFS_H

#include "vfs.h"
#include "list.h"

enum tmpfs_type { TREG, TDIR };

struct tmpfs_item {
  char name[16];
  enum tmpfs_type type;
  struct tmpfs_item *parent;
  size_t size;
  struct vnode* vn;
  union {
    char *content;
    struct list *children;
  } data;
};

int tmpfs_vn_rdwr(struct vnode* vn, struct uio *uiop, int rw);
int tmpfs_vn_lookup(struct vnode* vn, const char *name, struct vnode** dst);
int tmpfs_vn_create(struct vnode* vn, const char *name, struct vnode** dst);
int tmpfs_vn_mkdir(struct vnode* vn, const char *name, struct vnode** dst);
int tmpfs_vn_readdir(struct vnode *vn, struct uio *uiop);

int tmpfs_vfs_mount(struct vfs* vfsp, const char *path);
int tmpfs_vfs_root(struct vfs *vfsp, struct vnode **dst);

int tmpfs_create_vfs(struct vfs **dst, const char *name);

#endif
