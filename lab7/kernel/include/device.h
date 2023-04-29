#pragma once
#include <stdint.h>
#include "gpio.h"
#include "utils.h"
#include "vfs.h"
#include "uart.h"

#define MAX_DEVICE_IN_DIR 16

typedef enum { DEV_ROOT, DEV_UART, DEV_NONE, DEV_FB } DEV_TYPE;

struct device_fentry {
  char name[20];
  //FILE_TYPE type; // all FILE_REGULAR???
  DEV_TYPE type;
  struct vnode* vnode;
  struct vnode* parent_vnode;
  struct device_fentry* child[MAX_DEVICE_IN_DIR];
};

struct framebuffer_info {
  unsigned int width;
  unsigned int height;
  unsigned int pitch;
  unsigned int isrgb;
};

struct vnode_operations* device_v_ops;
struct file_operations* device_f_ops;

void device_init();
void device_set_fentry(struct device_fentry* fentry, const char* component_name, 
                      struct vnode* vnode, DEV_TYPE type);
int device_setup_mount(struct filesystem* fs, struct mount* mount);
int device_lookup(struct vnode* dir_node, struct vnode** target,
                 const char* component_name);
int device_create(struct vnode* dir_node, struct vnode** target,
                 const char* component_name, FILE_TYPE type);
int _device_create(struct vnode* dir_node, 
                 const char* component_name, DEV_TYPE type);
int device_set_parent(struct vnode* child_node, struct vnode* parent_vnode);
int device_write(struct file* file, const void* buf, size_t len);
int device_read(struct file* file, void* buf, size_t len);
void device_list(struct vnode* dir);

void root_init();

struct vnode_operations* uartfs_v_ops;
struct file_operations* uartfs_f_ops;
void uartfs_init();
int uartfs_read(struct file* file, void* buf, size_t len);
int uartfs_write(struct file* file, const void* buf, size_t len);


struct vnode_operations* fb_v_ops;
struct file_operations* fb_f_ops;
void fb_init();
int fb_write(struct file* file, const void* buf, size_t len);


//framebuffer syscall
long lseek64(int fd, long offset, int whence);
int ioctl(int fd, unsigned long request, struct framebuffer_info* info );
unsigned char *lfb;                       /* raw frame buffer address */
unsigned int isrgb; /* dimensions and channel order */
