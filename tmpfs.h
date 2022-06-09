#ifndef TMPFS_h
#define TMPFS_h
#include "vfs.h"

#define TMPFS_NAME_LEN 16
#define TMPFS_DIR_LEN 16
#define MAX_FILE_SIZE 4096
#define MODE_DIR 4
#define MODE_FILE 8

typedef struct File_Info{
	char name[TMPFS_NAME_LEN];
	int mode;
	unsigned long size;
	void* data;
} File_Info;

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount);
int initramfs_setup_mount(struct filesystem* fs, struct mount* mount);
vnode* vnode_create(vnode* parent, char* filename);

#endif