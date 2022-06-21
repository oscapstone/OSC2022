#include "vfs.h"
#include "mem.h"
#include "task.h"
#include "tmpfs.h"
#include "mmu.h"
#include "textio.h"
#include <string.h>
#include <errno.h>

struct vfs *root;
static char pathbuf[256];

int init_rootfs() {
  int ret = tmpfs_create_vfs(&root, "tmpfs");
  return ret;
}

int vfs_open(const char *fullpath, struct file **dst, int creat) {
  kprintf("[KVfs] vfs_open %s\n", fullpath);
  *dst = NULL;
  struct file *file = kmalloc(sizeof(struct file));
  if (file == NULL) {
    return -ENOMEM;
  }
  struct vnode *vn;
  int ret;
  if ((ret = vfs_lookup(fullpath, &vn)) < 0) {
    if (ret == -ENOENT && creat) {
      char *sepbuf = kmalloc(sizeof(pathbuf));
      if (sepbuf == NULL) {
        kfree(file);
        return -ENOMEM;
      }
      strncpy(sepbuf, fullpath, sizeof(pathbuf));
      char *pos, *cur;
      cur = sepbuf;
      while (*cur != '\0') {
        if (*cur == '/') pos = cur;
        cur++;
      }
      *pos = '\0';
      ret = vfs_lookup(sepbuf, &vn);
      kfree(sepbuf);
      if (ret == 0) {
        ret = vn->vn_ops->vn_create(vn, pos+1, &vn);
        if (ret == 0) {
          file->pos = 0;
          file->vn = vn;
          *dst = file;
          kprintf("[KVfs] created file %s 0x%lx\n", fullpath, *dst);
          return 0;
        }
      }
    }
    kprintf("[KVfs] vfs_open failed to open %s: %d\n", fullpath, ret);
    kfree(file);
    return ret;
  }
  file->pos = 0;
  file->vn = vn;
  *dst = file;
  return 0;
}

int vfs_close(struct file *file) {
  kprintf("[KVfs] vfs_close file 0x%lx\n", file);
  kfree(file);
  return 0;
}

int vfs_read(struct file *file, void* buf, size_t len, size_t *read_len) {
  kprintf("[KVfs] vfs_read %lx %ld %ld\n", file, buf, len, read_len);
  if (file->vn == NULL) return -EBADVN;
  struct uio io = { .buf = buf, .len = len, .off = file->pos, .ret = 0 };

  int ret = file->vn->vn_ops->vn_rdwr(file->vn, &io, UIO_READ);
  if (ret < 0) {
    return ret;
  }

  file->pos += io.ret;
  *read_len = io.ret;
  return 0;
}

int vfs_write(struct file *file, const void *buf, size_t len, size_t *written_len) {
  kprintf("[KVfs] vfs_write %lx %s %ld %ld\n", file, buf, len, written_len);
  if (file->vn == NULL) return -EBADVN;
  struct uio io = { .buf = (char*)buf, .len = len, .off = file->pos, .ret = 0 };

  int ret = file->vn->vn_ops->vn_rdwr(file->vn, &io, UIO_WRITE);
  if (ret < 0) {
    return ret;
  }

  file->pos += io.ret;
  *written_len = io.ret;
  return 0;
}

int vfs_mkdir(const char *fullpath) {
  kprintf("[KVfs] vfs_mkdir %s\n", fullpath);
  strncpy(pathbuf, fullpath, sizeof(pathbuf));
  char *pos, *cur = pathbuf;
  struct vnode *vn = NULL;
  while (*cur != '\0') {
    if (*cur == '/') pos = cur;
    cur++;
  }
  *pos = '\0';
  pos++;
  int ret = vfs_lookup(pathbuf, &vn);
  if (ret < 0) return ret;
  ret = vn->vn_ops->vn_mkdir(vn, pos, &vn);
  if (ret < 0) return ret;
  return 0;
}

int vfs_mount(const char *fullpath, const char *fsname) {
  kprintf("[KVfs] vfs_mount %s %s\n", fullpath, fsname);
  struct vfs *cur = root;
  while (cur != NULL) {
    if (strncmp(cur->vfs_name, fsname, sizeof(cur->vfs_name)) == 0) {
      cur->vfs_op->vfs_mount(cur, fullpath);
      break;
    }
  }
  if (cur == NULL) {
    return -ENOENT;
  }
  return 0;
}

int vfs_lookup(const char *fullpath, struct vnode **dst) {
  kprintf("[KVfs] vfs lookup %s\n", fullpath);
  strncpy(pathbuf, fullpath, sizeof(pathbuf));
  char *cur = strtok(pathbuf, VFS_DELIM);
  struct vnode *vn;
  int ret = root->vfs_op->vfs_root(root, &vn);
  if (ret < 0) return ret;

  while (cur != NULL) {
    ret = vn->vn_ops->vn_lookup(vn, cur, &vn);
    if (ret < 0) return ret;
    cur = strtok(NULL, VFS_DELIM);
  }
  *dst = vn;
  return 0;
}


int realpath(struct taskControlBlock *tsk, const char *relpath, char **dst) {
  *dst = NULL;
  char *path = (char*)kmalloc(256);
  if (path == NULL) return -ENOMEM;
  if (relpath[0] == '/') {
    strncpy(path, relpath, 256);
    *dst = path;
    return 0;
  }
  
  memset(path, 0, 256);
  memset(pathbuf, 0, sizeof(pathbuf));

  strncpy(pathbuf, tsk->pwd, sizeof(pathbuf));
  int len = strlen(pathbuf);
  if (len >= sizeof(pathbuf)) return -1;
  strncpy(pathbuf+len, relpath, sizeof(pathbuf)-len);

  char *cur = strtok(pathbuf, VFS_DELIM);
  char *pos = path;
  *pos++ = '/';
  
  while (cur != NULL) {
    if (strncmp(cur, "..", sizeof(pathbuf)) == 0) {
      int dcnt = 0;
      while (1) {
        if (*(pos-1) == '/') dcnt++;
        *pos = '\0';
        if (dcnt == 2) break;
        if (pos == path+1) break;
        else pos--;
      }
    } else if (strncmp(cur, ".", sizeof(pathbuf)) != 0) {
      while(*cur != '\0') {
        *pos = *cur;
        pos++; cur++;
      }
      *pos++ = '/';
    }
    cur = strtok(NULL, VFS_DELIM);
  }
  len = strlen(path);
  if (len > 1) path[len-1] = '\0';
  *dst = path;
  return 0;
}


int syscall_open(const char *pathname, int flags) {
  disable_preempt();
  int target = -1;
  for (int i = 0; i < 16; i++) {
    if (currentTask->fdtable[i] == NULL) {
      target = i; break;
    }
  }
  if (target < 0) {
    enable_preempt();
    return -ENOMEM;
  }
  
  struct file *f;
  char *fullpath;
  int ret;

  // get fullpath
  ret = realpath(currentTask, pathname, &fullpath);
  if (ret < 0) {
    enable_preempt();
    return ret;
  }

  // open file with fullpath
  ret = vfs_open(fullpath, &f, (flags & O_CREAT) ? 1 : 0);
  if (ret < 0) {
    kfree(fullpath);
    enable_preempt();
    return ret;
  }
  currentTask->fdtable[target] = f;

  enable_preempt();
  return target;
}

int syscall_close(int fd) {
  disable_preempt();
  if (currentTask->fdtable[fd] != NULL) {
    vfs_close(currentTask->fdtable[fd]);
    currentTask->fdtable[fd] = NULL;
  }
  enable_preempt();
  return 0;
}

long syscall_write(int fd, const void *buf, unsigned long count) {
  disable_preempt();
  struct file* f = currentTask->fdtable[fd];
  int ret;
  size_t wrt_len;
  if (f == NULL) { enable_preempt(); return -ENOENT; }
  
  ret = vfs_write(f, translate(currentTask, (uint64_t)buf), count, &wrt_len);
  
  enable_preempt();
  return ret >= 0 ? wrt_len : ret;
}

long syscall_read(int fd, void *buf, unsigned long count) {
  disable_preempt();
  struct file* f = currentTask->fdtable[fd];
  int ret;
  size_t rd_len;
  if (f == NULL) { enable_preempt(); return -ENOENT; }
  
  ret = vfs_read(f, translate(currentTask, (uint64_t)buf), count, &rd_len);
  
  enable_preempt();
  return ret >= 0 ? rd_len : ret;
}

int syscall_mkdir(const char *pathname, unsigned mode) {
  // ignore mode
  char *fullpath = NULL;
  int ret;
  char *pos, *cur;
  struct vnode *vn = NULL;
  disable_preempt();
  ret = realpath(currentTask, pathname, &fullpath);
  if (ret < 0) goto err;
  cur = fullpath;
  while (*cur != '\0') {
    if (*cur == '/') pos = cur;
    cur++;
  }
  *pos = '\0';
  pos++;
  ret = vfs_lookup(fullpath, &vn);
  if (ret < 0) goto err;
  ret = vn->vn_ops->vn_mkdir(vn, pos, &vn);
  if (ret < 0) goto err;
  enable_preempt();
  return 0;
 err:
  if (fullpath != NULL) kfree(fullpath);
  if (vn != NULL) kfree(vn);
  enable_preempt();
  return ret;
}

int syscall_mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data) {
  // ignore arguments other than target and filesystem
  int ret;
  char *fullpath = NULL;
  struct vfs *fs;
  disable_preempt();
  ret = realpath(currentTask, target, &fullpath);
  if (ret < 0) goto err;

  fs = root;
  while (fs != NULL) {
    if (strncmp(fs->vfs_name, filesystem, sizeof(fs->vfs_name)) == 0) {
      break;
    }
    fs = fs->vfs_next;
  }
  
  if (fs == NULL) {
    ret = -ENOENT;
    goto err;
  }

  ret = fs->vfs_op->vfs_mount(fs, fullpath);
  if (fs == NULL) goto err;
  kfree(fullpath);
  enable_preempt();
  return 0;
 err:
  if (fullpath != NULL) kfree(fullpath);
  enable_preempt();
  return ret;
}

int syscall_chdir(const char *path) {
  disable_preempt();
  
  char *fullpath = NULL;
  struct vnode* vn;
  int ret = realpath(currentTask, path, &fullpath);
  if (ret < 0) goto err;
  ret = vfs_lookup(fullpath, &vn);
  if (ret < 0) goto err;
  kfree(currentTask->pwd);

  // add / at the end
  int len = strlen(fullpath);
  char *newpwd = kmalloc(len+2);
  if (newpwd == NULL) goto err;
  strncpy(newpwd, fullpath, len+1);
  if (len > 1) {
    // exclude the case when the fullpath is "/"
    newpwd[len] = '/';
    newpwd[len+1] = '\0';
  }
  currentTask->pwd = newpwd;

  kfree(fullpath);
  enable_preempt();
  return 0;
 err:
  if (fullpath != NULL) kfree(fullpath);
  enable_preempt();
  return ret;
}


int vfs_register(struct vfs *fs) {
  struct vfs *cur = root;
  while (cur != NULL) {
    if (cur->vfs_next == NULL) {
      cur->vfs_next = fs;
      fs->vfs_next = NULL;
      return 0;
    }
    cur = cur->vfs_next;
  }
  return -1;
}
