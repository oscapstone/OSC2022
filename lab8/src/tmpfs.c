#include "tmpfs.h"
#include "vfs.h"
#include <string.h>
#include <errno.h>
#include "mem.h"

int tmpfs_vn_rdwr(struct vnode *vn, struct uio *uiop, int rw) {
  if (vn->vn_type != VREG) return -EACCESS;
  if (vn->vn_data == NULL) return -EBADVN;
  struct tmpfs_item *item = (struct tmpfs_item*)vn->vn_data;
  if (item->type != TREG) return -EBADIN;
  
  if (rw == UIO_READ) {
    if (uiop->len + uiop->off <= item->size) {
      memcpy(uiop->buf, item->data.content + uiop->off, uiop->len);
      uiop->ret = uiop->len;
    } else {
      if (uiop->off >= item->size) {
        uiop->ret = 0;
      } else {
        size_t len = item->size - uiop->off;
        memcpy(uiop->buf, item->data.content + uiop->off, len);
        uiop->ret = len;
      }
    }
  } else {
    size_t newsize = uiop->off + uiop->len;
    char *newcontent = kmalloc(newsize);
    if (newcontent == NULL) {
      return -ENOMEM;
    }
    if (item->data.content != NULL) {
      if (newsize < item->size) {
        memcpy(newcontent, item->data.content, newsize);
      } else {
        memcpy(newcontent, item->data.content, item->size);
      }
      kfree(item->data.content);
    }
    item->data.content = newcontent;
    item->size = newsize;
    
    memcpy(item->data.content + uiop->off, uiop->buf, uiop->len);
    uiop->ret = uiop->len;
  }
  return 0;
}

int tmpfs_vn_lookup(struct vnode *vn, const char *name, struct vnode **dst) {
  *dst = NULL;
  if (vn->vn_type != VDIR) return -ENOTDIR;
  if (vn->vn_mountedhere != NULL) {
    // this dir is a mount point do not treat as tmpfs
    struct vnode *rt;
    if (vn->vn_mountedhere->vfs_op->vfs_root(vn->vn_mountedhere, &rt) < 0) {
      return -EBADVN;
    }
    return rt->vn_ops->vn_lookup(rt, name, dst);
  }

  struct tmpfs_item *item = vn->vn_data;
  if (item->type != VDIR) return -EBADIN;
  if (item->data.children == NULL) return -ENOENT;
  struct listItem *item_c = item->data.children->first;
  while (item_c != NULL) {
    struct tmpfs_item *child = item_c->data;
    if (strncmp(child->name, name, sizeof(child->name)) == 0) {
      *dst = child->vn;
      break;
    }
    item_c = item_c->next;
  }
  if (*dst == NULL) return -ENOENT;
  return 0;
}

int tmpfs_vn_create(struct vnode *vn, const char *name, struct vnode **dst) {
  *dst = NULL;
  if (vn->vn_type != VDIR) return -ENOTDIR;
  if (vn->vn_mountedhere != NULL) {
    // this dir is a mount point do not treat as tmpfs
    struct vnode *rt;
    if (vn->vn_mountedhere->vfs_op->vfs_root(vn->vn_mountedhere, &rt) < 0) {
      return -EBADVN;
    }
    return rt->vn_ops->vn_create(rt, name, dst);
  }

  struct tmpfs_item *item = vn->vn_data;
  if (item->type != VDIR) return -EBADIN;
  
  if (item->data.children == NULL) { // if the dir has no child, create children list
    struct list *children = kmalloc(sizeof(struct list));
    if (children == NULL) return -ENOMEM;
    memset(children, 0, sizeof(struct list));
    item->data.children = children;
  }
  
  struct listItem *item_c = item->data.children->first;
  while (item_c != NULL) {
    struct tmpfs_item *child = item_c->data;
    if (strncmp(child->name, name, sizeof(child->name)) == 0) {
      *dst = child->vn;
      break;
    }
    item_c = item_c->next;
  }
  if (*dst != NULL) return -EEXIST;

  item_c = kmalloc(sizeof(struct listItem));
  struct tmpfs_item *new_item = kmalloc(sizeof(struct tmpfs_item));
  struct vnode *new_vn = kmalloc(sizeof(struct vnode));
  if (new_item == NULL || new_vn == NULL || item_c == NULL) {
    kfree(new_item);
    kfree(new_vn);
    kfree(item_c);
    return -ENOMEM;
  }

  *new_item = (struct tmpfs_item) {
    .parent = item,
    .size = 0,
    .type = TREG,
    .vn = new_vn
  };
  strncpy(new_item->name, name, sizeof(new_item->name));

  *new_vn = (struct vnode) {
    .vn_usecount = 0,
    .vn_type = VREG,
    .vn_ops = vn->vn_ops,
    .vn_vfs = vn->vn_vfs,
    .vn_mountedhere = NULL,
    .vn_data = new_item
  };

  item_c->size = sizeof(struct tmpfs_item);
  item_c->data = new_item;
  listAppend(item->data.children, item_c);

  *dst = new_vn;
  return 0;
}

int tmpfs_vn_mkdir(struct vnode *vn, const char *name, struct vnode **dst) {
  *dst = NULL;
  if (vn->vn_type != VDIR) return -ENOTDIR;
  if (vn->vn_mountedhere != NULL) {
    // this dir is a mount point do not treat as tmpfs
    struct vnode *rt;
    if (vn->vn_mountedhere->vfs_op->vfs_root(vn->vn_mountedhere, &rt) < 0) {
      return -EBADVN;
    }
    return rt->vn_ops->vn_mkdir(rt, name, dst);
  }

  struct tmpfs_item *item = vn->vn_data;
  if (item->type != VDIR) return -EBADIN;

  if (item->data.children == NULL) { // if the dir has no child, create children list
    struct list *children = kmalloc(sizeof(struct list));
    if (children == NULL) return -ENOMEM;
    memset(children, 0, sizeof(struct list));
    item->data.children = children;
  }
  
  
  struct listItem *item_c = item->data.children->first;
  while (item_c != NULL) {
    struct tmpfs_item *child = item_c->data;
    if (strncmp(child->name, name, sizeof(child->name)) == 0) {
      *dst = child->vn;
      break;
    }
    item_c = item_c->next;
  }
  if (*dst != NULL) return -EEXIST;

  struct tmpfs_item *new_item = kmalloc(sizeof(struct tmpfs_item));
  struct vnode *new_vn = kmalloc(sizeof(struct vnode));
  struct listItem *new_list_item = kmalloc(sizeof(struct listItem));
  if (new_item == NULL || new_vn == NULL || new_list_item == NULL) {
    kfree(new_item);
    kfree(new_vn);
    kfree(new_list_item);
    return -ENOMEM;
  }

  struct tmpfs_item ni = {
    .parent = item,
    .size = 0,
    .type = TDIR,
    .vn = new_vn
  };
  *new_item = ni;
  strncpy(new_item->name, name, sizeof(new_item->name));

  struct vnode nv = {
    .vn_usecount = 0,
    .vn_type = VDIR,
    .vn_ops = vn->vn_ops,
    .vn_vfs = vn->vn_vfs,
    .vn_mountedhere = NULL,
    .vn_data = new_item
  };
  *new_vn = nv;

  new_list_item->data = new_item;
  new_list_item->size = sizeof(struct tmpfs_item);

  listAppend(item->data.children, new_list_item);

  *dst = new_vn;
  return 0;
}

int tmpfs_vn_readdir(struct vnode *vn, struct uio *uiop) { return -1; }


int tmpfs_vfs_mount(struct vfs *vfsp, const char *path) {
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

int tmpfs_vfs_root(struct vfs *vfsp, struct vnode **dst) {
  *dst = NULL;
  struct tmpfs_item *tmpfs_rt = vfsp->vfs_data;
  if (tmpfs_rt == NULL) return -EBADIN;
  *dst = tmpfs_rt->vn;
  return 0;
}


int tmpfs_create_vfs(struct vfs **dst, const char *name) {
  *dst = NULL;
  struct vfs *tmpfs = kmalloc(sizeof(struct vfs));
  struct tmpfs_item *tmpfs_root = kmalloc(sizeof(struct tmpfs_item));
  struct vnode *vn_root = kmalloc(sizeof(struct vnode));
  struct vfsops *tmpfs_ops = kmalloc(sizeof(struct vfsops));
  struct vnodeops *tmpfs_vnops = kmalloc(sizeof(struct vnodeops));
  if (tmpfs == NULL || tmpfs_root == NULL || vn_root == NULL || tmpfs_ops == NULL || tmpfs_vnops == NULL) {
    kfree(tmpfs);
    kfree(tmpfs_root);
    kfree(vn_root);
    kfree(tmpfs_ops);
    kfree(tmpfs_vnops);
    return -ENOMEM;
  }

  *tmpfs = (struct vfs) { .vfs_next = NULL,
                          .vfs_op = tmpfs_ops,
                          .vfs_vncovered = NULL,
                          .vfs_data = tmpfs_root };
  strncpy(tmpfs->vfs_name, name, sizeof(tmpfs->vfs_name)); 
  *vn_root = (struct vnode) { .vn_type = VDIR,
                              .vn_usecount = 0,
                              .vn_ops = tmpfs_vnops,
                              .vn_vfs = tmpfs,
                              .vn_mountedhere = NULL,
                              .vn_data = tmpfs_root };
  *tmpfs_vnops = (struct vnodeops) { .vn_rdwr = tmpfs_vn_rdwr,
                                     .vn_lookup = tmpfs_vn_lookup,
                                     .vn_create = tmpfs_vn_create,
                                     .vn_mkdir = tmpfs_vn_mkdir,
                                     .vn_readdir = tmpfs_vn_readdir };

  *tmpfs_ops = (struct vfsops) { .vfs_mount = tmpfs_vfs_mount, .vfs_root = tmpfs_vfs_root };
  *tmpfs_root = (struct tmpfs_item) { .name = "",
                                      .type = TDIR,
                                      .parent = tmpfs_root,
                                      .size = 0,
                                      .vn = vn_root,
                                      .data.children = NULL };
  *dst = tmpfs;
  return 0;
}
