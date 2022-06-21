#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include "vfs.h"
#include "list.h"

#define PACKED __attribute__((packed))

struct PACKED fat32_bpb {
  uint8_t jump_boot[3];
  char oem_name[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t num_reserved_sectors;
  uint8_t num_fats;
  uint16_t num_root_entries; // 0 for FAT32
  uint16_t total_sectors_16; // 0 for fat32
  uint8_t media;             // 0xF8
  uint16_t fat_size_16;      // 0 for FAT32
  uint16_t sectors_per_track;
  uint16_t num_heads;
  uint32_t hidden_sectors;
  uint32_t total_sectors_32;
  uint32_t fat_size_32;
  uint16_t ext_flags;
  uint16_t fs_version;
  uint32_t root_cluster;
  uint16_t fs_info_sector;
  uint16_t backup_boot_sector;
  uint8_t reserved[12];
  uint8_t drive_number;
  uint8_t bs_reserved;
  uint8_t boot_signature;
  uint32_t volume_id;
  char volume_label[11];
  char fs_type[8];
}; // 90

struct PACKED fat32_fs_info {
  uint32_t lead_sig;
  uint8_t reserved[480];
  uint32_t struct_sig;
  uint32_t free_count;
  uint32_t next_free;
  uint8_t reserved2[12];
  uint8_t trail_sig[4];
}; // 512

struct PACKED fat32_dentry {
  char short_name[11];
  uint8_t attributes;
  uint8_t nt_res;
  uint8_t time_created_10ms;
  uint16_t time_created;
  uint16_t date_created;
  uint16_t date_last_access;
  uint16_t first_cluster_hi;
  uint16_t time_last_write; // extract these (pg29)
  uint16_t date_last_write;
  uint16_t first_cluster_lo;
  uint32_t file_size;
}; // 32


enum fat32_type { REG, DIR, OTH };

struct fat32_item {
  enum fat32_type type;
  struct vnode *vn;
  struct fat32_item *parent;
  struct list *children;
  char name[13];
  uint32_t ent_cluster, ent_offset;
  uint32_t data_cluster, data_offset;
};

struct fat32_data {
  struct fat32_bpb *bpb;
  struct fat32_fs_info *fsinfo;
  struct fat32_item *root_itm;
  uint32_t first_data_sector;
  uint32_t num_data_sectors;
  uint32_t num_data_clusters;
};


int fat32_vn_rdwr(struct vnode *vn, struct uio *uiop, int rw);
int fat32_vn_lookup(struct vnode* vn, const char *name, struct vnode** dst);
int fat32_vn_create(struct vnode* vn, const char *name, struct vnode** dst);
int fat32_vn_mkdir(struct vnode* vn, const char *name, struct vnode** dst);
int fat32_vn_readdir(struct vnode *vn, struct uio *uiop);

int fat32_vfs_mount(struct vfs* vfsp, const char *path);
int fat32_vfs_root(struct vfs *vfsp, struct vnode **dst);

int fat32_create_vfs(struct vfs **dst, const char *name);


#endif
