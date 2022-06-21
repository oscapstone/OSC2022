#include "fat32.h"
#include "sdhost.h"
#include "mem.h"
#include <string.h>
#include <textio.h>
#include <errno.h>

#define FAT32_BASE_BLK 2048
#define BYTES_PER_BLK 512

#define FAT32_ATTR_DIR 0x10
#define FSINFO_NXT_FREE_UNK 0xffffffff
#define FAT32_FAT_FREE 0x0
#define FAT32_FAT_END 0x0ffffff8
#define FAT32_FAT_BAD 0x0ffffff7

char fat32buf[4096];
char fat32buf2[4096];

struct fat32_bpb bpb;
struct fat32_fs_info fsinfo;
struct fat32_data *fsdata;


int load_sector(char *buf, uint32_t sector) {
  int blk_cnt = bpb.bytes_per_sector / BYTES_PER_BLK;
  for (int i = 0; i < blk_cnt; i++) {
    readblock(FAT32_BASE_BLK + sector * blk_cnt + i, buf + bpb.bytes_per_sector * i);
  }
  return 0;
}

int write_sector(char *buf, uint32_t sector) {
  int blk_cnt = bpb.bytes_per_sector / BYTES_PER_BLK;
  for (int i = 0; i < blk_cnt; i++) {
    writeblock(FAT32_BASE_BLK + sector * blk_cnt + i, buf + bpb.bytes_per_sector * i);
  }
  return 0;
}

uint32_t ith_sector_of_cluster(uint32_t cluster, int i) {
  return ((cluster - 2) * bpb.sectors_per_cluster) + fsdata->first_data_sector + i;
}


int fat32_create_vfs(struct vfs **dst, const char *name) {
  struct vfs *fs = kmalloc(sizeof(struct vfs));
  struct fat32_data *data = kmalloc(sizeof(struct fat32_data));
  struct vnode *vnrt = kmalloc(sizeof(struct vnode));
  struct fat32_item *itmrt = kmalloc(sizeof(struct fat32_item));
  struct vnodeops *vnops = kmalloc(sizeof(struct vnodeops));
  struct vfsops *fsops = kmalloc(sizeof(struct vfsops));

  if (fs == NULL || data == NULL || vnrt == NULL || itmrt == NULL ||
      vnops == NULL || fsops == NULL) {
    kfree(fs);
    kfree(data);
    kfree(vnrt);
    kfree(itmrt);
    kfree(vnops);
    kfree(fsops);
    return -ENOMEM;
  }

  *vnops = (struct vnodeops) {
    .vn_rdwr = fat32_vn_rdwr,
    .vn_lookup = fat32_vn_lookup,
    .vn_create = fat32_vn_create,
    .vn_mkdir = fat32_vn_mkdir,
    .vn_readdir = fat32_vn_readdir
  };

  *fsops = (struct vfsops) {
    .vfs_mount = fat32_vfs_mount,
    .vfs_root = fat32_vfs_root
  };

  *vnrt = (struct vnode) {
    .vn_type = VDIR,
    .vn_usecount = 0,
    .vn_ops = vnops,
    .vn_vfs = fs,
    .vn_mountedhere = NULL,
    .vn_data = itmrt
  };

  
  readblock(FAT32_BASE_BLK, &fat32buf);
  memcpy(&bpb, fat32buf, sizeof(bpb));
  readblock(FAT32_BASE_BLK+1, &fat32buf);
  memcpy(&fsinfo, fat32buf, sizeof(fsinfo));
  
  data->first_data_sector = bpb.num_reserved_sectors + bpb.num_fats * bpb.fat_size_32;
  data->num_data_sectors = bpb.total_sectors_32 - bpb.num_reserved_sectors - bpb.num_fats * bpb.fat_size_32;
  data->num_data_clusters = data->num_data_sectors / bpb.sectors_per_cluster;
  data->root_itm = itmrt;
  data->bpb = &bpb;
  data->fsinfo = &fsinfo;

  fsdata = data;

  *itmrt = (struct fat32_item) {
    .type = DIR,
    .vn = vnrt,
    .parent = NULL,
    .children = NULL,
    .name = "",
    .ent_cluster = -1,
    .ent_offset = -1,
    .data_cluster = bpb.root_cluster,
    .data_offset = 0
  };
  
  *fs = (struct vfs) {
    .vfs_next = NULL,
    .vfs_op = fsops,
    .vfs_vncovered = NULL,
    .vfs_data = data
  };

  *dst = fs;

  return 0;
}

uint32_t next_cluster(uint32_t curr_cluster) {
  static char fat[4096];
  if (curr_cluster >= bpb.total_sectors_32) return 0x0fffffff;
  
  uint32_t fat_n = curr_cluster / (bpb.bytes_per_sector / 4);
  load_sector(fat, bpb.num_reserved_sectors + fat_n);

  uint32_t val = *(uint32_t*)&(fat[(curr_cluster * 4) % bpb.bytes_per_sector]);
  return val & 0x0fffffff;
}

int fat32_vn_rdwr(struct vnode *vn, struct uio *uiop, int rw) {
  if (vn->vn_type != VREG) return -EACCESS;
  if (vn->vn_data == NULL) return -EBADVN;
  struct fat32_item *item = (struct fat32_item*)vn->vn_data;
  if (item->type != REG) return -EBADIN;

  if (rw == UIO_READ) {
    kprintf("[fat32] read %s 0x%x %d\n", item->name, uiop->off, uiop->len); 
    struct fat32_dentry ent;
    load_sector(fat32buf, ith_sector_of_cluster(item->ent_cluster, item->ent_offset / bpb.bytes_per_sector));
    memcpy(&ent, fat32buf+item->ent_offset % bpb.bytes_per_sector, sizeof(ent));
    uint32_t size = ent.file_size;
    uint32_t cluster_size = bpb.bytes_per_sector * bpb.sectors_per_cluster;
    uint32_t off = uiop->off;
    uint32_t cur_cluster = item->data_cluster;
    uint32_t ith_cluster  = off / cluster_size;
    uint32_t read_size = 0;
    uint32_t remain_size = uiop->len;
    kprintf("name: %s, size: %d\n", ent.short_name, ent.file_size);
    /* adjust size to avoid out of bound error */
    if (off >= size) {
      uiop->ret = 0;
      return 0;
    } else if (off + remain_size > size) {
      remain_size = size - off;
    }
    
    for (int i = 0; i < ith_cluster; i++) {
      cur_cluster = next_cluster(cur_cluster);
      if (cur_cluster >= 0x0ffffff7) break; 
    }

    if (cur_cluster >= 0x0ffffff7) {
      uiop->ret = 0;
      return -1;
    }

    off = off % cluster_size;
    while (remain_size > 0) {
      for (int i = off / bpb.bytes_per_sector; i < bpb.sectors_per_cluster; i++) {
        load_sector(fat32buf, ith_sector_of_cluster(cur_cluster, i));
        if (remain_size >= bpb.bytes_per_sector) {
          memcpy(uiop->buf + read_size, fat32buf, bpb.bytes_per_sector);
          read_size += bpb.bytes_per_sector;
          remain_size -= bpb.bytes_per_sector;
        } else {
          memcpy(uiop->buf + read_size, fat32buf, remain_size);
          read_size += remain_size;
          remain_size = 0;
        }
      }
      cur_cluster = next_cluster(cur_cluster);
      if (cur_cluster >= 0x0ffffff7) break;
      off = 0;
    }
    
    uiop->ret = read_size;
    return 0;
  } else {

    
    
    uiop->ret = 0;
    return -EACCESS;
  }
  return 0;
}

static char to_lower(char alpha) {
  if (alpha >= 'A' && alpha <= 'Z')
    return alpha - 'A' + 'a';
  return alpha;
}

static char to_upper(char alpha) {
  if (alpha >= 'a' && alpha <= 'z')
    return alpha - 'a' + 'A';
  return alpha;
}

int fat32_vn_lookup(struct vnode* vn, const char *name, struct vnode** dst){
  *dst = NULL;
  if (vn->vn_type != VDIR) return -ENOTDIR;
  if (vn->vn_mountedhere != NULL) {
    struct vnode *rt;
    if (vn->vn_mountedhere->vfs_op->vfs_root(vn->vn_mountedhere, &rt) < 0) {
      return -EBADVN;
    }
    return rt->vn_ops->vn_lookup(rt, name, dst);
  }

  char lowername[16];
  strncpy(lowername, name, 15);
  lowername[15] = 0;
  for (int i = 0; i < 16; i++) {
    lowername[i] = to_lower(lowername[i]);
  }

  struct fat32_item *item = vn->vn_data;
  if (item->type != DIR) return -EBADIN;
  if (item->children != NULL) {
    struct listItem *itr = item->children->first;
    while (itr != NULL) {
      struct fat32_item *child = itr->data;
      if (strncmp(lowername, child->name, 16) == 0) {
        *dst = child->vn;
        return 0;
      }
      itr = itr->next;
    }
    return -ENOENT;
  }

  kprintf("[fat32] lookup dir children list is NULL\n");
  struct list *children = kmalloc(sizeof(struct list));
  if (children == NULL) return -ENOMEM;
  memset(children, 0, sizeof(struct list));
  item->children = children;

  uint32_t cur_cluster = item->data_cluster;
  while (1) {
    int endent = 0;
    for (int i = 0; i < bpb.sectors_per_cluster; i++) {
      load_sector(fat32buf, ith_sector_of_cluster(cur_cluster, i));
      struct fat32_dentry *ent = (struct fat32_dentry*)fat32buf;
      for (int j = 0; j < bpb.bytes_per_sector / sizeof(struct fat32_dentry); j++) {
        if (ent[j].short_name[0] == 0) {
          endent = 1;
          break;
        }

        struct fat32_item *new_itm = kmalloc(sizeof(struct fat32_item));
        struct vnode *new_vn = kmalloc(sizeof(struct vnode));
        struct listItem *li = kmalloc(sizeof(struct listItem));
        if (new_itm == NULL || new_vn == NULL) {
          kfree(new_itm);
          kfree(new_vn);
          kfree(li);
          return -ENOMEM;
        }

        /* setup file type */
        if (ent[j].attributes & FAT32_ATTR_DIR) {
          new_itm->type = DIR;
          new_vn->vn_type = VDIR;
        } else {
          new_itm->type = REG;
          new_vn->vn_type = VREG;
        }

        new_vn->vn_mountedhere = NULL;
        new_vn->vn_usecount = 0;
        new_vn->vn_ops = vn->vn_ops;
        new_vn->vn_vfs = vn->vn_vfs;
        new_vn->vn_data = new_itm;

        new_itm->vn = new_vn;
        new_itm->parent = item;
        new_itm->children = NULL;
        new_itm->data_cluster = ((uint32_t)ent[j].first_cluster_hi << 16) | (ent[j].first_cluster_lo);
        new_itm->data_offset = 0;
        new_itm->ent_cluster = cur_cluster;
        new_itm->ent_offset = i * bpb.bytes_per_sector + j * sizeof(struct fat32_dentry);
        int k, k1;
        for (k = 0; k < 8; k++) {
          char ch = ent[j].short_name[k];
          if (ch == ' ') {
            break;
          }
          new_itm->name[k] = to_lower(ch);
        }
        new_itm->name[k] = '.';
        k1 = k+1;
        for (k = 0; k < 3; k++) {
          char ch = ent[j].short_name[8+k];
          if (ch == ' ') {
            new_itm->name[k1+k] = 0;
            break;
          }
          new_itm->name[k1+k] = to_lower(ch);
        }
        new_itm->name[k1+3] = 0;

        li->data = new_itm;
        li->size = sizeof(struct fat32_item);
        li->next = li->prev = NULL;
        listAppend(item->children, li);
      }
      if (endent) break;
    }
    
    if (endent) break;
    cur_cluster = next_cluster(cur_cluster);
    if (cur_cluster >= 0x0ffffff7) break;
  }
  
  return fat32_vn_lookup(vn, name, dst);
}

uint32_t next_free_cluster(uint32_t cur_cluster) {
  static char fat[4096];
  if (cur_cluster >= bpb.total_sectors_32) return FSINFO_NXT_FREE_UNK;
  
  uint32_t fat_n = cur_cluster / (bpb.bytes_per_sector / 4);
  load_sector(fat, bpb.num_reserved_sectors + fat_n);

  while (1) {
    uint32_t i = (cur_cluster * 4) % bpb.bytes_per_sector;
    uint32_t fat_val = *(uint32_t*)&(fat[i]);
    if ((fat_val & 0x0fffffff) == FAT32_FAT_FREE) {
      return cur_cluster;
    }
    cur_cluster++;
    if (cur_cluster % (bpb.bytes_per_sector / 4) == 0) {
      fat_n = cur_cluster / (bpb.bytes_per_sector / 4);
      load_sector(fat, bpb.num_reserved_sectors + fat_n);
    }
    if (cur_cluster >= bpb.total_sectors_32) return FSINFO_NXT_FREE_UNK;
  }
}

uint32_t extend_cluster_chain(uint32_t cur_cluster) {
  static char fat[4096];
  if (fsinfo.next_free == FSINFO_NXT_FREE_UNK) {
    fsinfo.next_free = next_free_cluster(bpb.root_cluster);
  } else {
    fsinfo.next_free = next_free_cluster(fsinfo.next_free);
  }
  uint32_t free_cluster = fsinfo.next_free;
  fsinfo.free_count--;
  writeblock(FAT32_BASE_BLK+1, &fsinfo);

  uint32_t fat_n = cur_cluster / (bpb.bytes_per_sector / 4);
  load_sector(fat, bpb.num_reserved_sectors + fat_n);

  uint32_t i = (cur_cluster * 4) % bpb.bytes_per_sector;
  *(uint32_t*)&(fat[i]) = free_cluster & 0x0fffffff;
  write_sector(fat, bpb.num_reserved_sectors + fat_n);

  fat_n = free_cluster / (bpb.bytes_per_sector / 4);
  load_sector(fat, bpb.num_reserved_sectors + fat_n);
  i = (free_cluster * 4) % bpb.bytes_per_sector;
  *(uint32_t*)&(fat[i]) = FAT32_FAT_END;
  write_sector(fat, bpb.num_reserved_sectors + fat_n);
  return free_cluster;
}

int fat32_vn_create(struct vnode *vn, const char *name, struct vnode **dst) {
  *dst = NULL;
  if (vn->vn_type != VDIR) return -EACCESS;
  if (vn->vn_mountedhere != NULL) {
    struct vnode *mount_vn;
    int ret = vn->vn_mountedhere->vfs_op->vfs_root(vn->vn_mountedhere, &mount_vn);
    if (ret < 0) return ret;
    return mount_vn->vn_ops->vn_create(mount_vn, name, dst);
  }

  struct fat32_item *item = vn->vn_data;
  if (item->type != DIR) return -EBADIN;

  int k, k1;


  struct vnode *new_vn = kmalloc(sizeof(struct vnode));
  struct fat32_item *new_item = kmalloc(sizeof(struct vnode));
  struct listItem *li = kmalloc(sizeof(struct listItem));
  if (new_vn == NULL || new_item == NULL || li == NULL) {
    kfree(new_item); kfree(new_vn); kfree(li);
    return -ENOMEM;
  }

  // find a free cluster
  uint32_t file_cluster;
  if (fsinfo.next_free == FSINFO_NXT_FREE_UNK) {
    file_cluster = next_free_cluster(bpb.root_cluster);
  } else {
    file_cluster = next_free_cluster(fsinfo.next_free);
  }
  fsinfo.next_free = file_cluster;
  fsinfo.free_count--;
  writeblock(FAT32_BASE_BLK+1, &fsinfo);

  uint32_t fat_n = file_cluster / (bpb.bytes_per_sector / 4);
  load_sector(fat32buf2, bpb.num_reserved_sectors + fat_n);

  uint32_t idx = (file_cluster * 4) % bpb.bytes_per_sector;
  *(uint32_t*)&(fat32buf2[idx]) = FAT32_FAT_END;
  write_sector(fat32buf2, bpb.num_reserved_sectors + fat_n);

  // setup fat32 short name style name
  char new_name[11];
  memset(new_name, ' ', sizeof(new_name));

  int i;
  for (i = 0; i < 8; i++) {
    if (name[i] == '.') break;
    if (name[i] == '\0') break;
    new_name[i] = to_upper(name[i]);
  }
  i++;
  for (int j = i; j < i+3; j++) {
    if (name[i] == '\0') break;
    new_name[8+j-i] = to_upper(name[j]);
  }

  kprintf("fatname: %s\n", new_name);
  kprintf("loaction: 0x%x\n", file_cluster);

  // create a direntry
  uint32_t cur_cluster = item->data_cluster;
  int mark_zero = 0;
  while (1) {
    for (int i = 0; i < bpb.sectors_per_cluster; i++) {
      load_sector(fat32buf, ith_sector_of_cluster(cur_cluster, i));
      struct fat32_dentry *ent = (struct fat32_dentry*)fat32buf;
      if (mark_zero) {
        ent[0].short_name[0] = 0;
        write_sector(fat32buf, ith_sector_of_cluster(cur_cluster, i));
        goto created;
      }
      for (int j = 0; j < bpb.bytes_per_sector / sizeof(struct fat32_dentry); j++) {
        if (ent[j].short_name[0] == 0) {
          // create a entry here
          
          memcpy(ent[j].short_name, new_name, sizeof(new_name));
          ent[j].file_size = 0;
          ent[j].attributes = 0;
          ent[j].first_cluster_hi = file_cluster >> 16;
          ent[j].first_cluster_lo = file_cluster & 0xffff;

          new_item->data_cluster = file_cluster;
          new_item->data_offset = 0;
          new_item->ent_cluster = cur_cluster;
          new_item->ent_offset = j * sizeof(struct fat32_dentry) + i * bpb.bytes_per_sector;


          if (j < bpb.bytes_per_sector / sizeof(struct fat32_dentry) - 1) {
            ent[j+1].short_name[0] = 0;
            write_sector(fat32buf, ith_sector_of_cluster(cur_cluster, i));
            goto created;
          } else {
            mark_zero = 1;
          }
        }
      }
    }
    uint32_t nxt = next_cluster(cur_cluster);
    if (nxt >= FAT32_FAT_END) {
      nxt = extend_cluster_chain(cur_cluster);
    }
    cur_cluster = nxt;
  }
  
 created:
  for (k = 0; k < 8; k++) {
    char ch = new_name[k];
    if (ch == ' ') {
      break;
    }
    new_item->name[k] = to_lower(ch);
  }
  new_item->name[k] = '.';
  k1 = k+1;
  for (k = 0; k < 3; k++) {
    char ch = new_name[8+k];
    if (ch == ' ') {
      new_item->name[k1+k] = 0;
      break;
    }
    new_item->name[k1+k] = to_lower(ch);
  }
  new_item->name[k1+3] = 0;
  new_item->children = NULL;
  new_item->parent = item;
  new_item->type = REG;
  new_item->vn = new_vn;

  new_vn->vn_vfs = vn->vn_vfs;
  new_vn->vn_data = new_item;
  new_vn->vn_mountedhere = NULL;
  new_vn->vn_ops = vn->vn_ops;
  new_vn->vn_usecount = 0;
  new_vn->vn_type = VREG;

  li->data = new_item;
  li->size = sizeof(struct fat32_item);
  li->next = li->prev = NULL;
  listAppend(item->children, li);

  *dst = new_vn;
  
  return 0;
}


int fat32_vn_mkdir(struct vnode* vn, const char *name, struct vnode** dst){ return -1; }

int fat32_vn_readdir(struct vnode *vn, struct uio *uiop){ return -1; }

int fat32_vfs_mount(struct vfs* vfsp, const char *path){
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

int fat32_vfs_root(struct vfs *vfsp, struct vnode **dst) {
  struct fat32_data *data = vfsp->vfs_data;
  if (data->root_itm == NULL) {
    *dst = NULL;
    return -EBADIN;
  }
  *dst = data->root_itm->vn;
  return 0;
}

