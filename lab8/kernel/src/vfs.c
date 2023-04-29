#include "vfs.h"

#include "alloc.h"
#include "cpio.h"
#include "fatfs.h"
#include "printf.h"
#include "string.h"
#include "thread.h"
#include "tmpfs.h"
#include "device.h"

void vfs_test() {
  const char* argv[] = {"vfs_test", 0};
  exec("vfs_test", argv);
}

void vfs_ls_test() {
  const char* argv[] = {"ls", ".", 0};
  exec("ls", argv);
}

void vfs_hard_test() {
  const char* argv[] = {"vfs_test_hard", 0};
  exec("vfs_test_hard", argv);
}

void vfs_fat_test() {
  const char* argv[] = {"fatfs_test", 0};
  exec("fatfs_test", argv);
}

void vnode_print( struct vnode* vnode){
  struct tmpfs_fentry* fentry = (struct tmpfs_fentry*) vnode->internal;
  printf("[vnode_print]name = %s\n", fentry->name);
}

void vfs_init() {
  fs_list.head = 0;
  fs_list.tail = 0;
  // init and register tmpfs
  tmpfs_init();
  struct filesystem* tmpfs =
      (struct filesystem*)malloc(sizeof(struct filesystem));
  tmpfs->name = "tmpfs";
  tmpfs->setup_mount = tmpfs_setup_mount;
  register_filesystem(tmpfs);

  // use tmpfs to mount root filesystem
  rootfs = (struct mount*)malloc(sizeof(struct mount));
  struct filesystem* fs = get_fs_by_name("tmpfs");
  if (fs == 0) {
    // printf("[Error] Mount root filesystem failed!!\n");
    return;
  }
  fs->setup_mount(fs, rootfs);
  current_dir = rootfs->root;
  // cpio_populate_rootfs();
  
  // init and register cpio
  cpiofs_init();
  struct filesystem* cpiofs =
      (struct filesystem*)malloc(sizeof(struct filesystem));
  cpiofs->name = "cpiofs";
  cpiofs->setup_mount = cpiofs_setup_mount;
  register_filesystem(cpiofs);

  vfs_mkdir("/initramfs");
  vfs_mount("", "/initramfs", "cpiofs");

  //device
  device_init();
  struct filesystem* devfs =
      (struct filesystem*)malloc(sizeof(struct filesystem));
  devfs->name = "dev";
  devfs->setup_mount = device_setup_mount;
  register_filesystem(devfs);

  vfs_mkdir("/dev");
  vfs_mount("", "/dev", "dev");

  // init and register fatfs
  fatfs_init();
  struct filesystem* fatfs =
      (struct filesystem*)malloc(sizeof(struct filesystem));
  fatfs->name = "fatfs";
  fatfs->setup_mount = fatfs_setup_mount;
  register_filesystem(fatfs);

  vfs_mkdir("/boot");
  vfs_mount("", "/boot", "fatfs");
}

int register_filesystem(struct filesystem* fs) {
  // register the file system to the kernel.
  if (fs_list.head == 0) {
    fs_list.head = fs;
    fs_list.head->next = 0;
    fs_list.tail = fs_list.head;
  } else {
    fs_list.tail->next = fs;
    fs_list.tail = fs_list.tail->next;
  }
  return 1;
}

struct filesystem* get_fs_by_name(const char* name) {
  for (struct filesystem* fs = fs_list.head; fs != 0; fs = fs->next) {
    if (!strcmp(fs->name, name)) {
      return fs;
    }
  }
  // printf("[Error] Filesystem \'%s\' not found!!\n", name);
  return 0;
}

int vfs_find_vnode(struct vnode** target, const char* pathname) {
  if (!strcmp(pathname, "/")) {
    *target = rootfs->root;
    return 1;
  }

  char* pathname_ = (char*)malloc(strlen(pathname) + 1);
  strcpy(pathname_, pathname);
  struct vnode* dir = current_dir;
  if (pathname_[0] == '/') dir = rootfs->root;
  // printf("[vfs_find_vnode]find node: %s\n", pathname_);

  char* component_name = strtok(pathname_, '/');
  while (component_name && *component_name != '\0') {
    int found = dir->v_ops->lookup(dir, target, component_name);
    // printf("[vfs_find_vnode]component name: %s, found %d\n", component_name, found);
    if (!found) {
      printf("[Error] No such file or directory: %s\n", pathname);
      free(pathname_);
      return 0;
    }
    if ((*target)->mount) *target = (*target)->mount->root;
    dir = *target;
    component_name = strtok(0, '/');
  }
  free(pathname_);
  return 1;
}

struct file* vfs_open(const char* pathname, int flags) {
  // 1. Lookup pathname from the root vnode.
  // 2. Create a new file descriptor for this vnode if found.
  // 3. Create a new file if O_CREAT is specified in flags.
  struct vnode* dir = current_dir;
  struct vnode* target = 0;
  struct file* file = 0;

  char* pathname_ = (char*)malloc(strlen(pathname) + 1);
  strcpy(pathname_, pathname);
  // pathname: /mnt       -> pathname_: "\0",  filename: mnt
  // pathname: /mnt/file1 -> pathname_: /mnt,  filename: file1
  // pathname: file1      -> pathname_: file1, filename: NULL
  char* filename = split_last(pathname_, '/');
  // printf("[vfs_open] pathname_ :%s, filename :%s\n", pathname_, filename);
  if (*pathname_ == '\0' && pathname[0] == '/') {
    dir = rootfs->root;
  }
  if (filename != 0) {
    int prefix_found = vfs_find_vnode(&dir, pathname_);
    // e.g., given pathname /abc/zxc/file1, but /abc/zxc not found
    if (!prefix_found) {
      printf("[Error] Path does not exist: %s\n", pathname);
      free(pathname_);
      return 0;
    }
  } else {
    filename = pathname_;
  }

  int file_found = dir->v_ops->lookup(dir, &target, filename);
  if (flags == O_CREAT) {
    printf("[open]CREATE\n");
    if (!file_found) {
      dir->v_ops->create(dir, &target, filename, FILE_REGULAR);
      file = (struct file*)malloc(sizeof(struct file));
      file->vnode = target;
      file->f_ops = target->f_ops;
      file->f_pos = 0;
    } else {
      printf("[Error] File already exists: %s\n", pathname);
    }
  } else {
    if (!file_found) {
      printf("[Error] File does not exist: %s\n", pathname);
    } else {
      if (target->mount) target = target->mount->root;
      file = (struct file*)malloc(sizeof(struct file));
      file->vnode = target;
      file->f_ops = target->f_ops;
      file->f_pos = 0;
    }
  }
  free(pathname_);
  return file;
}

int vfs_close(struct file* file) {
  // 1. release the file descriptor
  // free(file);
  return 1;
}

int vfs_write(struct file* file, const void* buf, size_t len) {
  // 1. write len byte from buf to the opened file.
  // 2. return written size or error code if an error occurs.
  return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, size_t len) {
  // 1. read min(len, readable file data size) byte to buf from the opened file.
  // 2. return read size or error code if an error occurs.
  return file->f_ops->read(file, buf, len);
}

int vfs_list(struct file* file, void* buf, int index) {
  return file->f_ops->list(file, buf, index);
}

int vfs_mkdir(const char* pathname) {
  // printf("%s\n", pathname);
  struct vnode* dir = current_dir;
  struct vnode* target = 0;

  char* pathname_ = (char*)malloc(strlen(pathname) + 1);
  strcpy(pathname_, pathname);
  // pathname: /mnt      -> pathname_: "\0",  dirname: mnt
  // pathname: /mnt/dir1 -> pathname_: /mnt,  dirname: dir1
  // pathname: dir1      -> pathname_: dir1,  dirname: NULL
  char* dirname = split_last(pathname_, '/');
  printf("[vfs_mkdir] pathname_ :%s, dirname :%s\n", pathname_, dirname);

  if (*pathname_ == '\0' && pathname[0] == '/') {
    dir = rootfs->root;
  }

  if (dirname != 0) {
    int prefix_found = vfs_find_vnode(&dir, pathname_);
    // e.g., given pathname /abc/zxc/file1, but /abc/zxc not found
    if (!prefix_found) {
      // printf("[Error] Path does not exist: %s\n", pathname);
      free(pathname_);
      return 0;
    }
  } else {
    dirname = pathname_;
  }

  int file_found = dir->v_ops->lookup(dir, &target, dirname);
  if (file_found) {
    printf("[Error] File already exists: %s\n", pathname);
    return 0;
  }
  int status = dir->v_ops->create(dir, &target, dirname, FILE_DIRECTORY);
  free(pathname_);
  return status;
}

int vfs_chdir(const char* pathname) {
  struct vnode* target = 0;
  int dir_found = vfs_find_vnode(&target, pathname);
  if (!dir_found) {
    // printf("[Error] Directory does not exist: %s\n", pathname);
    return 0;
  }
  vnode_print(current_dir);
  vnode_print(target);
  current_dir = target;
  // struct tmpfs_fentry* fentry = (struct tmpfs_fentry*)current_dir->internal;
  // printf("[chdir] %s\n", fentry->name);
  return 1;
}

int vfs_mount(const char* device, const char* mountpoint,
              const char* filesystem) {
  printf("[vfs_mount] %s\n", mountpoint);
  struct vnode* target = 0;
  int dir_found = vfs_find_vnode(&target, mountpoint);

  if (!dir_found) {
    printf("[Error] Directory does not exist: %s\n", mountpoint);
    return 0;
  }
  if (target->mount) {
    printf("[Error] This directory has been mounted: %s\n", mountpoint);
    return 0;
  }

  // find parent_vnode
  char* pathname_ = (char*)malloc(strlen(mountpoint) + 5);
  strcpy(pathname_, mountpoint);
  struct vnode* parent_vnode = 0;
  strcat(pathname_, "/..");
  // int parent_found = vfs_find_vnode(&parent_vnode, pathname_);
  vfs_find_vnode(&parent_vnode, pathname_);
  free(pathname_);

  struct mount* mountfs = (struct mount*)malloc(sizeof(struct mount));
  struct filesystem* fs = get_fs_by_name(filesystem);
  fs->setup_mount(fs, mountfs);

  //target is mountpoint
  target->mount = mountfs;
  mountfs->root->v_ops->set_parent(mountfs->root, parent_vnode);
  return 1;
}

int vfs_umount(const char* mountpoint) {
  // printf("[umount] %s\n", mountpoint);
  struct vnode* dir = current_dir;
  struct vnode* target = 0;

  char* pathname_ = (char*)malloc(strlen(mountpoint) + 1);
  strcpy(pathname_, mountpoint);
  // pathname: /mnt      -> pathname_: "\0",  dirname: mnt
  // pathname: /mnt/dir1 -> pathname_: /mnt,  dirname: dir1
  // pathname: dir1      -> pathname_: dir1,  dirname: NULL
  char* dirname = split_last(pathname_, '/');
  if (*pathname_ == '\0' && mountpoint[0] == '/') {
    dir = rootfs->root;
  }
  // printf("%s %s\n", pathname_, dirname);

  if (dirname != 0) {
    int prefix_found = vfs_find_vnode(&dir, pathname_);
    // e.g., given pathname /abc/zxc/file1, but /abc/zxc not found
    if (!prefix_found) {
      // printf("[Error] Path does not exist: %s\n", mountpoint);
      free(pathname_);
      return 0;
    }
  } else {
    dirname = pathname_;
  }

  int file_found = dir->v_ops->lookup(dir, &target, dirname);
  if (!file_found) {
    // printf("[Error] Directory does not exist: %s\n", mountpoint);
    return 0;
  }

  if (!target->mount) {
    // printf("[Error] This directory is not mounted: %s\n", mountpoint);
    return 0;
  }
  target->mount = 0;
  return 1;
}