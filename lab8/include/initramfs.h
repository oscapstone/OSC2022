#ifndef INITRAMFS_H
#define INITRAMFS_H

#include "vfs.h"
#include "list.h"
#include "cpio.h"

enum initrd_type { IREG, IDIR };

struct initrd_item {
  enum initrd_type type;
  struct vnode *vn;
  struct initrd_item *parent;
  char *name;
  struct cpio_newc_header *file_header;
  struct list *children;
};

struct initrd_fs_data {
  struct cpio_newc_header *first;
  struct vnode *root;
};

int initrd_vn_rdwr(struct vnode* vn, struct uio *uiop, int rw);
int initrd_vn_lookup(struct vnode* vn, const char *name, struct vnode** dst);
int initrd_vn_create(struct vnode* vn, const char *name, struct vnode** dst);
int initrd_vn_mkdir(struct vnode* vn, const char *name, struct vnode** dst);
int initrd_vn_readdir(struct vnode *vn, struct uio *uiop);

int initrd_vfs_mount(struct vfs* vfsp, const char *path);
int initrd_vfs_root(struct vfs *vfsp, struct vnode **dst);

int initrd_create_vfs(struct vfs **dst, const char *name, uint64_t addr);


#endif
