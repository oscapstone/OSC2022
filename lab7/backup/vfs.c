#include "vfs.h"
#include "mem.h"
#include <string.h>

struct vfs *rootfs;
static char path_buf[256];

#define DELIM "/"

int vfs_open(const char *fullpath, struct vfs_file **target, int flag) {
  struct vfs_file *file = kmalloc(sizeof(struct vfs_file));
  if (file == NULL) return -1;
  file->fullpath = kmalloc(strlen(fullpath)+1);
  if (file->fullpath == NULL) {
    goto err;
  }
  
  strncpy(path_buf, fullpath, sizeof(path_buf));
  char *cur = strtok(path_buf, DELIM);
  char *nxt;
  struct vnode* vn = rootfs->vfs_vnodecovered;
  while ((nxt = strtok(NULL, DELIM)) != NULL) {
    if (vn->v_type != VDIR) goto err;
    if (vn->v_dir_children == NULL)
      vn->v_ops->vn_lookup(vn, cur, &vn);
    else {
      struct listItem *itr = vn->v_dir_children->first;
    }
    cur = nxt;
  }
  if (vn->v_type != VDIR) goto err;
  
  struct vnode* fvn;
  if (vn->v_ops->vn_lookup(vn, cur, &fvn) == 0 ||
      (flag && vn->v_ops->vn_create(vn, cur, &fvn) == 0)) {
    file->pos = 0;
    file->vn = fvn;
    *target = file;
    return 0;
  }
 err:
  if (file->fullpath != NULL) kfree(file->fullpath);
  kfree(file);
  *target = NULL;
  return -1;
}

int vfs_close(struct vfs_file *file) {
  kfree(file->fullpath);
  kfree(file);
  return 0;
}

int vfs_lseek(struct vfs_file *file) {
  return -1;
}

int vfs_mkdir(const char *fullpath) {  
  strncpy(path_buf, fullpath, sizeof(path_buf));
  char *cur = strtok(path_buf, DELIM);
  char *nxt;
  struct vnode* vn = rootfs->vfs_vnodecovered;
  while ((nxt = strtok(NULL, DELIM)) != NULL) {
    if (vn->v_type != VDIR) goto err;
    vn->v_ops->vn_lookup(vn, cur, &vn);
    cur = nxt;
  }
  if (vn->v_type != VDIR) goto err;
  
  struct vnode* fvn;
  if (vn->v_ops->vn_lookup(vn, cur, &fvn) == 0) return -1;
    return 0;
  }
 err:
  if (file->fullpath != NULL) kfree(file->fullpath);
  kfree(file);
  *target = NULL;
  return -1;
}

int vfs_remove(const char *fullpath) { return 0; }

int vfs_mount(const char *mount_path, struct vfs *fs) { return 0; }

int vfs_lookup(const char *fullpath, struct vnode **target) { return 0; }

int register_filesystem(struct vfs* fs) {
  if (rootfs == NULL) {
    rootfs = fs;
    return 0;
  } else {
    struct vfs *itr = rootfs;
    while (itr->vfs_next != NULL) {
      itr = itr->vfs_next;
    }
    itr->vfs_next = fs;
    return 0;
  }
}
