#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vfs.h>
#include "utils.h"

#define RAMFS_ADDR 0x8000000
#define CPIO_MAGIC "070701"
#define CPIO_END "TRAILER!!!"

#define MAX_FILES_IN_DIR 16
#define CPIOFS_BUF_SIZE (500 * kb)

typedef struct {
  char c_magic[6];
  char c_ino[8];
  char c_mode[8];
  char c_uid[8];
  char c_gid[8];
  char c_nlink[8];
  char c_mtime[8];
  char c_filesize[8];
  char c_devmajor[8];
  char c_devminor[8];
  char c_rdevmajor[8];
  char c_rdevminor[8];
  char c_namesize[8];
  char c_check[8];
} cpio_newc_header;

void cpio_ls();
void cpio_cat(char *filename);
uint32_t cpio_load_user_program(const char *target_program,
                                uint64_t target_addr);
void cpio_populate_rootfs();

struct cpiofs_file {
  size_t size;
  char *buffer;
};

struct cpiofs_fentry {
  char name[20];
  FILE_TYPE type;
  struct vnode* vnode;
  struct vnode* parent_vnode;
  struct cpiofs_fentry* child[MAX_FILES_IN_DIR];
  struct cpiofs_file* file;
};

struct vnode_operations* cpiofs_v_ops;
struct file_operations* cpiofs_f_ops;

void cpiofs_init();
void cpiofs_set_fentry(struct cpiofs_fentry* fentry, const char* component_name,
                      struct cpiofs_file* file, FILE_TYPE type, struct vnode* vnode);
int cpiofs_setup_mount(struct filesystem* fs, struct mount* mount);
int cpiofs_lookup(struct vnode* dir_node, struct vnode** target,
                 const char* component_name);
int cpiofs_create(struct vnode* dir_node, struct vnode** target,
                 const char* component_name, FILE_TYPE type);
int cpiofs_set_parent(struct vnode* child_node, struct vnode* parent_vnode);
int cpiofs_write(struct file* file, const void* buf, size_t len);
int cpiofs_read(struct file* file, void* buf, size_t len);
int cpiofs_list(struct file* file, void* buf, int index);
