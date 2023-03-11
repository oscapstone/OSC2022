#include "list.h"
#include "vfs.h"
#include "tmpfs.h"
#include "mem.h"
#include <string.h>

struct vnodeops tmpfsVNodeOps = {.vn_mkdir = tmpfs_vn_mkdir,
                                 .vn_create = tmpfs_vn_create,
                                 .vn_lookup = tmpfs_vn_lookup,
                                 .vn_remove = tmpfs_vn_remove,
                                 .vn_read = tmpfs_vn_read,
                                 .vn_write = tmpfs_vn_write};

struct tmpfsItem* tmpfsCreateFile(struct tmpfsItem *item, const char *filename) {
  if (item->type != Dir) {
    return NULL;
  }

  struct list *children = &item->entry.dir->children;
  struct listItem *it = children->first;
  while (it != NULL) {
    struct tmpfsItem *child = it->data;
    if (child->type == File) {
      if (strncmp(child->entry.file->name, filename, 256) == 0) {
        return child;
      }
    } else {
      if (strncmp(child->entry.dir->dirname, filename, 256) == 0) {
        return NULL;
      }
    }
    it = it->next;
  }

  struct tmpfsFile *newFile = (struct tmpfsFile*)kmalloc(sizeof(struct tmpfsFile));
  newFile->size = 0;
  size_t len = strlen(filename);
  newFile->name = (char*)kmalloc(len+1);
  memcpy(newFile->name, filename, len+1);

  struct tmpfsItem *newItem = (struct tmpfsItem*)kmalloc(sizeof(struct tmpfsItem));
  newItem->type = File;
  newItem->entry.file = newFile;

  struct listItem *newListItem = (struct listItem*)kmalloc(sizeof(struct listItem));
  newListItem->data = newItem;
  newListItem->size = sizeof(struct tmpfsItem);

  listAppend(children, newListItem);
  return newItem;
}

struct tmpfsItem* tmpfsMakeDir(struct tmpfsItem *item, const char *dirname) {
  if (item->type != Dir) {
    return NULL;
  }

  struct list *children = &item->entry.dir->children;
  struct listItem *it = children->first;
  while (it != NULL) {
    struct tmpfsItem *child = it->data;
    if (child->type == File) {
      if (strncmp(child->entry.file->name, dirname, 256) == 0) {
        return NULL;
      }
    } else {
      if (strncmp(child->entry.dir->dirname, dirname, 256) == 0) {
        return child;
      }
    }
    it = it->next;
  }


  struct tmpfsDir *newDir = (struct tmpfsDir*)kmalloc(sizeof(struct tmpfsDir));
  size_t len = strlen(dirname);
  newDir->dirname = (char*)kmalloc(len+1);
  memcpy(newDir->dirname, dirname, len+1);
  newDir->parent = item;
  memset(&newDir->children, 0, sizeof(struct list));
  
  struct tmpfsItem *newItem = (struct tmpfsItem*)kmalloc(sizeof(struct tmpfsItem));
  newItem->type = Dir;
  newItem->entry.dir = newDir;

  struct listItem *newListItem = (struct listItem*)kmalloc(sizeof(struct listItem));
  newListItem->data = newItem;
  newListItem->size = sizeof(struct tmpfsItem);

  listAppend(children, newListItem);
  return newItem;
}


size_t tmpfsWrite(struct tmpfsFile *file, const char *buf, size_t size, uint32_t offset) {
  if (offset + size > file->size) {
    char *newContent = (char*)kmalloc(size + offset);
    if (newContent == NULL) return -1;
    memcpy(newContent, file->content, file->size);
    file->size = offset + size;
    kfree(file->content);
  }
  memcpy(file->content+offset, buf, size);
  return size;
}

size_t tmpfsRead(struct tmpfsFile *file, char *buf, size_t size, uint32_t offset) {
  if (file->size < offset) return 0;
  if (file->size < offset + size) {
    size = file->size - offset;
  }
  memcpy(buf, file->content + offset, size);  
  return size;
}

int tmpfsRmDir(struct tmpfsItem *dir_item) {
  if (dir_item->type != Dir) return -1;
  struct tmpfsDir *dir = dir_item->entry.dir;
  if (dir->children.itemCount > 0) {
    return -2;
  }
  struct tmpfsDir *parent = dir->parent->entry.dir;
  struct listItem *itr = parent->children.first;
  while (itr != NULL) {
    if (itr->data == dir_item) {
      listRemoveItem(&parent->children, itr);
      kfree(itr);
      break;
    }
    itr = itr->next;
  }
  kfree(dir_item->entry.dir);
  kfree(dir_item);
  return 0;
}

int tmpfsRmFile(struct tmpfsItem *dir_item, struct tmpfsItem *file_item) {
  struct tmpfsDir *dir = dir_item->entry.dir;
  struct tmpfsFile *file = file_item->entry.file;

  struct list *children = &dir->children;
  struct listItem *itr = children->first;
  int match = 0;
  while (itr != NULL) {
    if (itr->data == file_item) {
      listRemoveItem(children, itr);
      kfree(itr);
      match = 1;
      break;
    }
    itr = itr->next;
  }
  if (!match) {
    return -1;
  }

  kfree(file->content);
  kfree(file->name);
  kfree(file);
  kfree(file_item);
  return 0;
}


void tmpfs_vn_freelist(struct list *lst) {
  struct listItem *elm = lst->first;
  struct vnode *elm_vn;
  while (elm != NULL) {
    elm_vn = elm->data;
    kfree(elm_vn);
    elm = elm->next;
  }

  elm = lst->first;
  struct listItem* prev = elm->prev;
  while (elm != NULL) {
    elm = elm->next;
    kfree(prev);
  }
  kfree(lst);
}

int tmpfs_vn_readdir(struct vnode* vn_dir) {
  if (vn_dir->v_dir_children == NULL) {
    vn_dir->v_dir_children = (struct list*)kmalloc(sizeof(struct list));
    if (vn_dir->v_dir_children == NULL) return -1;
    struct tmpfsDir *dir = ((struct tmpfsItem*)vn_dir->v_data)->entry.dir;
    struct listItem *itr = dir->children.first;
    while (itr != NULL) {
      struct vnode *new_vn = (struct vnode*)kmalloc(sizeof(struct vnode));
      if (new_vn == NULL) {
        tmpfs_vn_freelist(vn_dir->v_dir_children);
        vn_dir->v_dir_children = NULL;
        return -1;
      }
      
      new_vn->v_mountedvfs = NULL;
      new_vn->v_parent_vfs = vn_dir->v_parent_vfs;
      new_vn->v_ops = &tmpfsVNodeOps;
      if (((struct tmpfsItem*)itr->data)->type == Dir) {
        new_vn->v_type = VDIR;
      } else {
        new_vn->v_type = VFILE;
      }
      new_vn->v_data = itr->data;
      
      struct listItem *item = (struct listItem*)kmalloc(sizeof(struct listItem));
      if (item == NULL) {
        kfree(new_vn);
        tmpfs_vn_freelist(vn_dir->v_dir_children);
        vn_dir->v_dir_children = NULL;
        return -1;
      }
      
      item->next = NULL;
      item->prev = NULL;
      item->data = new_vn;
      item->size = sizeof(struct vnode);
    
      listAppend(vn_dir->v_dir_children, item);
      itr = itr->next;
    }
    return 0;
  }
  return 1;
}

int tmpfs_vn_mkdir(struct vnode* vn_dir, const char *name, struct vnode** target) {
  if (tmpfs_vn_readdir(vn_dir) < 0) return -2;
  
  struct tmpfsItem *dir_item = (struct tmpfsItem*)vn_dir->v_data;
  struct tmpfsItem *new_dir_item = tmpfsMakeDir(dir_item, name);
  if (new_dir_item == NULL) {
    *target = NULL;
    return -1;
  }

  struct vnode* new_vnode = (struct vnode*)kmalloc(sizeof(struct vnode)); 
  if (new_vnode == NULL) {
    tmpfsRmDir(new_dir_item);
    *target = NULL;
    return -1;
  }
  new_vnode->v_mountedvfs = NULL;
  new_vnode->v_parent_vfs = vn_dir->v_parent_vfs;
  new_vnode->v_ops = &tmpfsVNodeOps;
  new_vnode->v_type = VDIR;
  new_vnode->v_data = new_dir_item;

  struct listItem* new_vn_lst_item = (struct listItem*)kmalloc(sizeof(struct listItem));
  if (new_vn_lst_item == NULL) {
    tmpfsRmDir(new_dir_item);
    kfree(new_vnode);
    *target = NULL;
    return -1;
  }
  listAppend(vn_dir->v_dir_children, new_vn_lst_item);
  
  *target = new_vnode;  
  return 0;
}

int tmpfs_vn_create(struct vnode* vn_dir, const char *name, struct vnode** target) {
  struct tmpfsItem *dir_item = (struct tmpfsItem*)vn_dir->v_data;
  struct tmpfsItem *new_file_item = tmpfsCreateFile(dir_item, name);
  if (new_file_item == NULL) {
    *target = NULL;
    return -1;
  }

  struct vnode* new_vnode = (struct vnode*)kmalloc(sizeof(struct vnode));
  if (new_vnode == NULL) {
    *target = NULL;
    return -1;
  }
  new_vnode->v_mountedvfs = NULL;
  new_vnode->v_parent_vfs = vn_dir->v_parent_vfs;
  new_vnode->v_ops = &tmpfsVNodeOps;
  new_vnode->v_type = VFILE;
  new_vnode->v_data = new_file_item;

  *target = new_vnode;  
  return 0;
}

int tmpfs_vn_lookup(struct vnode* vn_dir, const char *name, struct vnode** target) {
  if (vn_dir->v_dir_children != NULL) return -2;
  if (tmpfs_vn_readdir(vn_dir) < 0) return -2;

  struct listItem* itr = vn_dir->v_dir_children->first;
  struct vnode *match = NULL;
  while (itr != NULL) {
    struct vnode *vn = (struct vnode*)itr->data;
    struct tmpfsItem *it = vn->v_data;
    switch (it->type) {
    case Dir:
      if (strncmp(it->entry.dir->dirname, name, 256) == 0) {
        match = vn;
      }
      break;
    case File:
      if (strncmp(it->entry.file->name, name, 256) == 0) {
        match = vn;
      }
      break;
    }
    if (match) {
      break;
    }
    itr = itr->next;
  }
  if (match) {
    *target = match;
    return 0;
  }
  *target = NULL;
  return -1;
}

int tmpfs_vn_remove(struct vnode *vn_dir, const char *name) {
  struct list *children = vn_dir->v_dir_children;
  struct listItem *itr = children->first;
  while (itr != NULL) {
    struct vnode* vn = itr->data;
    struct tmpfsItem *itm = vn->v_data;
    int match = 0;
    if (itm->type == Dir) {
      if (strncmp(itm->entry.dir->dirname, name, 256) == 0) {
        tmpfsRmDir(itm);
        match = 1;
      }
    } else if (itm->type == File) {
      if (strncmp(itm->entry.file->name, name, 256) == 0) {
        tmpfsRmFile(vn_dir->v_data, itm);
        match = 1;
      }
    }
    if (match) {
      listRemoveItem(children, itr);
      kfree(itr->data);
      kfree(itr);
      return 0;
    } else {
      itr = itr->next;
    }
  }
  return -1;
}

int tmpfs_vn_write(struct vnode *vn_file, struct uio *uiop) {
  struct tmpfsItem *item = vn_file->v_data;
  struct tmpfsFile *file = item->entry.file;
  if (!file) return -1;
  uiop->ret = tmpfsWrite(file, uiop->buf, uiop->len, uiop->off);
  if (uiop->ret != uiop->len) return -1;
  return 0;
}

int tmpfs_vn_read(struct vnode* vn_file, struct uio *uiop) {
  struct tmpfsItem *item = vn_file->v_data;
  struct tmpfsFile *file = item->entry.file;
  if (!file) return -1;
  uiop->ret = tmpfsRead(file, uiop->buf, uiop->len, uiop->off);
  return 0;
}


int tmpfs_createfs(struct vfs **target) {
  
  struct vfs *vfsp = (struct vfs*)kmalloc(sizeof(struct vfs));
  struct tmpfsItem *root_item = kmalloc(sizeof(struct tmpfsItem));
  struct tmpfsDir *root_dir = kmalloc(sizeof(struct tmpfsDir));
  struct vnode *root_vn = kmalloc(sizeof(struct vnode));
  char *dirname = kmalloc(1);

  if (!vfsp || !root_item || !root_dir || !root_vn || !dirname) {
    kfree(vfsp); 
    kfree(root_item);
    kfree(root_dir);
    kfree(root_vn);
    kfree(dirname);
    *target = NULL;
    return -1;
  }

  *target = vfsp;
  
  dirname[0] = '\0';
  root_dir->dirname = dirname;
  root_dir->parent = root_item; // the parent of a root is itself
  memset(&root_dir->children, 0, sizeof(struct list));
  
  root_item->type = Dir;
  root_item->entry.dir = root_dir;
  
  vfsp->vfs_next = NULL;
  vfsp->vfs_data = root_item;

  if (root_vn == NULL) {
    kfree(*target);
    kfree(root_dir->dirname);
    kfree(root_dir);
    kfree(root_item);
    *target = NULL;
    return -1;
  }

  root_vn->v_parent_vfs = vfsp;
  root_vn->v_type = VDIR;
  root_vn->v_mountedvfs = NULL;
  root_vn->v_data = root_item;
  root_vn->v_ops = &tmpfsVNodeOps;
  root_vn->v_dir_children = NULL;

  vfsp->vfs_vnodecovered = root_vn;
  
  return 0;
}
