#include "initramfs.h"
#include "vfs.h"
#include "mem.h"
#include "textio.h"
#include <cpio.h>
#include <errno.h>
#include <string.h>

int initrd_vn_rdwr(struct vnode* vn, struct uio *uiop, int rw) {
  if (vn->vn_type != VREG) return -EACCESS;
  if (rw == UIO_WRITE) return -EACCESS;
  
  struct initrd_item *item = vn->vn_data;
  if (item->type != IREG) return -EBADVN;

  size_t read_len = cpio_read(item->file_header, uiop->off, uiop->buf, uiop->len);
  uiop->ret = read_len;
  return 0;
}

char* pathpushfront(char *dst, const char *src) {
  int org_len = strlen(dst);
  int add_len = strlen(src);
  for (int i = org_len + add_len - 1; i >= add_len; i--) {
    dst[i] = dst[i-add_len];
  }
  for (int i = 0; i < add_len-1; i++) dst[i] = src[i];
  return dst;
}

int initrd_item_free(struct initrd_item *itm) {
  if (itm->type == IDIR) {
    struct listItem *li = itm->children->first;
    struct listItem *nli; 
    while (li != NULL) {
      nli = li->next;
      kfree(li->data);
      kfree(li);
      li = nli;
    }
    kfree(itm->children);
    kfree(itm->vn);
    kfree(itm->name);
    kfree(itm);
  } else {
    kfree(itm->vn);
    kfree(itm->name);
    kfree(itm);
  }
  return 0;
}

int initrd_create_item(struct cpio_newc_header *p_header, struct initrd_item* parent, struct initrd_item **dst) {
  static char namebuf[256];
  cpio_filename(p_header, namebuf, sizeof(namebuf));
  char *last = namebuf, *itr = namebuf;
  while (*itr != '\0') {
    if (*itr == '/') last = itr+1;
    itr++;
  }

  *dst = NULL;

  int namelen = strlen(last);
  char *name = kmalloc(namelen+1);
  struct vnode *vn = kmalloc(sizeof(struct vnode));
  struct initrd_item *item = kmalloc(sizeof(struct initrd_item));
  if (name == NULL || item == NULL || vn == NULL) {
    kfree(name);
    kfree(item);
    kfree(vn);
    return -ENOMEM;
  }

  strncpy(name, last, namelen+1);

  
  uint64_t mode = cpio_mode(p_header);
  uint64_t type = (mode >> 12) & 15;

  item->children = NULL;
  item->parent = parent;
  item->name = name;
  item->file_header = p_header;
  item->vn = vn;

  vn->vn_data = item;
  vn->vn_mountedhere = NULL;
  vn->vn_ops = parent->vn->vn_ops;
  vn->vn_usecount = 0;
  vn->vn_vfs = parent->vn->vn_vfs;

  
  if (type == 4) {
    item->type = IDIR;
    vn->vn_type = VDIR;
  } else if (type == 8) {
    item->type = IREG;
    vn->vn_type = VREG;
  } else {
    kprintf("[initrdfs] Unsupported file type\n");
  }

  *dst = item;
  
  return 0;
}

int initrd_vn_lookup(struct vnode* vn, const char *name, struct vnode** dst) {
  kprintf("[initrdfs] lookup %x %s\n", vn, name);
  static char pathbuf[256];
  static char namebuf[256];
  int ret;
  *dst = NULL;
  if (vn->vn_type != VDIR) return -EACCESS;
  if (vn->vn_mountedhere != NULL) {
    struct vnode *mount_vn;
    ret = vn->vn_mountedhere->vfs_op->vfs_root(vn->vn_mountedhere, &mount_vn);
    if (ret < 0) return ret;
    return mount_vn->vn_ops->vn_lookup(mount_vn, name, dst);
  }

  struct initrd_item *item = vn->vn_data;
  if (item->children != NULL) {
    struct listItem *itr = item->children->first;
    while (itr != NULL) {
      struct initrd_item *child = itr->data;
      if (strncmp(name, child->name, 256) == 0) {
        *dst = child->vn;
        return 0;
      }
      itr = itr->next;
    }
    return -ENOENT;
  }

  kprintf("[initrdfs] lookup dir children list is NULL\n");

  struct list *children = kmalloc(sizeof(struct list));
  if (children == NULL) return -ENOMEM;

  memset(children, 0, sizeof(struct list));
  memset(pathbuf, 0, sizeof(pathbuf));

  item->children = children;

  struct initrd_item *parent = item;
  while (parent != NULL) {
    pathpushfront(pathbuf, "/");
    // kprintf("parent: '%s' '%s'\n", parent->name, pathbuf);
    pathpushfront(pathbuf, parent->name);
    parent = parent->parent;
  }

  // kprintf("prefix %s\n", pathbuf+1);
  
  struct initrd_fs_data *fsdata = vn->vn_vfs->vfs_data;
  struct cpio_newc_header *f = fsdata->first;

  size_t pathlen = strlen(pathbuf);
  while (f != NULL) {
    cpio_filename(f, namebuf, sizeof(namebuf));
    // compare filename prefix
    if (strncmp(namebuf, pathbuf, pathlen) == 0) {      
      char *suffix_itr = namebuf + pathlen;
      while (*suffix_itr != '\0') {
        if (*suffix_itr == '/') break;
        suffix_itr++;
      }
      if (*suffix_itr == '\0') {
        struct initrd_item *child;
        ret = initrd_create_item(f, item, &child);
        if (ret < 0) {
          kprintf("[initrdfs] Error creating child node\n");
          f = cpio_nextfile(f);
          continue;
        }

        struct listItem *new_li = kmalloc(sizeof(struct listItem));
        if (new_li == NULL) {
          kprintf("[initrdfs] Out of mem when creating listItem\n");
          initrd_item_free(child);
          continue;
        }

        new_li->data = child;
        new_li->size = sizeof(struct initrd_item);
        listAppend(item->children, new_li);
      }
    }
    f = cpio_nextfile(f);
  }

  return initrd_vn_lookup(vn, name, dst);
}


int initrd_vn_create(struct vnode* vn, const char *name, struct vnode** dst) {
  *dst = NULL;

  if (vn->vn_mountedhere != NULL) {
    struct vnode *mount_vn;
    int ret = vn->vn_mountedhere->vfs_op->vfs_root(vn->vn_mountedhere, &mount_vn);
    if (ret < 0) return ret;
    return mount_vn->vn_ops->vn_create(mount_vn, name, dst);
  }
  
  return -EACCESS;
}

int initrd_vn_mkdir(struct vnode* vn, const char *name, struct vnode** dst) {
  *dst = NULL;
  
  if (vn->vn_mountedhere != NULL) {
    struct vnode *mount_vn;
    int ret = vn->vn_mountedhere->vfs_op->vfs_root(vn->vn_mountedhere, &mount_vn);
    if (ret < 0) return ret;
    return mount_vn->vn_ops->vn_mkdir(mount_vn, name, dst);
  }
  
  return -EACCESS;
}


int initrd_vn_readdir(struct vnode *vn, struct uio *uiop) {
  return -1;
}

int initrd_vfs_mount(struct vfs *vfsp, const char *path) {
  static char fullpath[256];
  strncpy(fullpath, path, sizeof(fullpath));
  char *cur = strtok(fullpath, VFS_DELIM);
  struct vnode *vn;
  int ret = 0;
  if ((ret = root->vfs_op->vfs_root(root, &vn)) < 0) {
    return ret;
  }
  while (cur != NULL) {
    ret = vn->vn_ops->vn_lookup(vn, cur, &vn);
    if (ret < 0) {
      return ret;
    }
    cur = strtok(NULL, VFS_DELIM);
  }
  if (vn->vn_type != VDIR) {
    return -ENOTDIR;
  }
  vfsp->vfs_vncovered = vn;
  vn->vn_mountedhere = vfsp;
  return 0;
  
}

int initrd_vfs_root(struct vfs *vfsp, struct vnode **dst) {
  struct initrd_fs_data *data = vfsp->vfs_data;
  if (data->root == NULL) {
    *dst = NULL;
    return -EBADIN;
  }
  *dst = data->root;
  return 0;
}

int initrd_create_vfs(struct vfs **dst, const char *name, uint64_t addr) {
  struct vfs *fs = kmalloc(sizeof(struct vfs));
  struct initrd_fs_data *data = kmalloc(sizeof(struct initrd_fs_data));
  struct vnode *vnrt = kmalloc(sizeof(struct vnode));
  struct initrd_item *itmrt = kmalloc(sizeof(struct initrd_item));
  struct vnodeops *vnops = kmalloc(sizeof(struct vnodeops));
  struct vfsops *fsops = kmalloc(sizeof(struct vfsops));
  char *namert = kmalloc(1);

  if (fs == NULL || data == NULL || vnrt == NULL || itmrt == NULL ||
      vnops == NULL || fsops == NULL || namert == NULL) {
    kfree(fs);
    kfree(data);
    kfree(vnrt);
    kfree(itmrt);
    kfree(vnops);
    kfree(fsops);
    kfree(namert);
    return -ENOMEM;
  }

  namert[0] = '\0';

  *vnops = (struct vnodeops) {
    .vn_rdwr = initrd_vn_rdwr,
    .vn_lookup = initrd_vn_lookup,
    .vn_create = initrd_vn_create,
    .vn_mkdir = initrd_vn_mkdir,
    .vn_readdir = initrd_vn_readdir
  };

  *fsops = (struct vfsops) {
    .vfs_mount = initrd_vfs_mount,
    .vfs_root = initrd_vfs_root
  };

  *itmrt = (struct initrd_item) {
    .type = IDIR,
    .vn = vnrt,
    .parent = NULL,
    .name = namert,
    .file_header = cpio_first(addr),
    .children = NULL
  };

  *vnrt = (struct vnode) {
    .vn_type = VDIR,
    .vn_usecount = 0,
    .vn_ops = vnops,
    .vn_vfs = fs,
    .vn_mountedhere = NULL,
    .vn_data = itmrt
  };

  data->first = cpio_first(addr);
  data->root = vnrt;

  *fs = (struct vfs) {
    .vfs_next = NULL,
    .vfs_op = fsops,
    .vfs_vncovered = NULL,
    .vfs_data = data
  };

  *dst = fs;

  return 0;
}
