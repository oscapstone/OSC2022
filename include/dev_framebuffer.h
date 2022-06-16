#ifndef H_DEV_FRAMEBUFFER
#define H_DEV_FRAMEBUFFER

#include "vfs.h"

extern file_operations_t dev_framebuffer_file_operations;
extern unsigned int width, height, pitch, isrgb;

typedef struct framebuffer_info {
  unsigned int width;
  unsigned int height;
  unsigned int pitch;
  unsigned int isrgb;
} framebuffer_info_t;

void init_dev_framebuffer();
int dev_framebuffer_write(file_t *file, const void *buf, size_t len);
int dev_framebuffer_open(vnode_t *file_node, file_t **target);
int dev_framebuffer_close(file_t *file);

#endif