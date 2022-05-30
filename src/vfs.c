#include "vfs.h"
#include "tmpfs.h"
#include "utils.h"
#include "allocator.h"
#include "string.h"
#include "mini_uart.h"
#include "sched.h"
#include "syscall.h"
struct mount* rootfs;
extern Thread_struct* get_current();
extern int open(char *pathname,int flags);
extern void ls();
extern int mkdir(char* pathname);
void testfs_exec(){
  rootfs_init("tmpfs");
  Thread_struct* cur_thread = get_current();
  asm volatile(
      "mov x0, 0\n\t" // 
      "msr spsr_el1, x0\n\t"
      "msr elr_el1, %0\n\t"
      "msr sp_el0,%1\n\t"
      "mov sp, %2\n\t"
      "msr tpidr_el1, %3\n\t"
      ::"r" (test_fs),
      "r" ((char*)(cur_thread->user_stack+THREAD_STACK_SIZE)),
      "r" ((char*)(cur_thread->kernel_stack+THREAD_STACK_SIZE)),
      "r" ((char*)cur_thread)
      : "x0"
  );
  
  asm volatile(
      "eret\n\t" // 
  );
}
void test_fs()
{
  open("/abc",0);
  open("/a",O_CREAT);
  open("/b",O_CREAT);
  open("/c",O_CREAT);
  ls();
  mkdir("/abc");
  ls();
  while(1){};
  return;
}

void rootfs_init(char* name)
{
  
  struct filesystem* rfs = (struct filesystem*)my_malloc(sizeof(struct filesystem));
  strcpy((char*)(rfs->name),name);
  
  if(strcmp(name,"tmpfs")==0){
    rfs->setup_mount = tmpfs_setup_mount;
  }
  else{
    busy_wait_writes("[*]No file system mounted, got ",FALSE);
    busy_wait_writes(name,TRUE);
  }
  
  if(register_filesystem(rfs)==-1)
  {
    busy_wait_writes("[*] Register file system failed.",TRUE);
    free(rfs);
    return;
  }

  rootfs = (struct mount*)my_malloc(sizeof(struct mount));
  rfs->setup_mount(rfs,rootfs);
  get_current()->pwd = rootfs->root;
  return;
}

int register_filesystem(struct filesystem* fs) {
  // register the file system to the kernel.
  // you can also initialize memory pool of the file system here.
  if(strcmp(fs->name,"tmpfs")==0)
  {
      return tmpfs_register();
  }
  return -1;
}

int vfs_open(const char* pathname, int flags, struct file** target) {
  
  // 1. Lookup pathname
  struct file* target_file = my_malloc(sizeof(struct file));
  struct vnode *v_node;
  // struct vnode *v_dir,*v_node;
  // v_dir = rootfs->root;
  // int ret = rootfs->root->v_ops->lookup(v_dir,&v_node,pathname);
  int ret = vfs_lookup(pathname,&v_node);
  // v_node = target_file->vnode;
  // 2. Create a new file handle for this vnode if found.
  if(ret == sucessMsg)
  {
    target_file->f_pos = 0;
    target_file->f_ops = v_node->f_ops;
    target_file->vnode = v_node;
    target_file->flags = flags;
    writes_uart_debug("[*]Opening file",FALSE);
    writes_uart_debug((char*)pathname,TRUE);
  }
  else if(flags == O_CREAT && ret == lastCompNotFound){
    
    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    char buf[COMP_NAME_LEN];
    int i,j;
    for(i=0,j=0;i<strlen((char*)pathname);i++) // buf = component name
    {
      if(pathname[i]=='/'){
        buf[0]='\0';
        j=0;
      }else{
        buf[j++] = pathname[i];
      }
    }
    buf[j] = '\0';
    writes_uart_debug("[*]Start create file in dir: ",FALSE);
    writes_uart_debug(((struct tmpfs_inode*)(v_node->internal))->name,TRUE);
    struct vnode *new_vnode;
    
    v_node->v_ops->create(v_node,&new_vnode,buf);
    target_file->f_pos = 0;
    target_file->f_ops = new_vnode->f_ops;
    target_file->vnode = new_vnode;
    target_file->flags = flags;
    writes_uart_debug("[*]File: ",FALSE);
    writes_uart_debug(buf,FALSE);
    writes_uart_debug(" has been created",TRUE);
  }
  else{
    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails
    writes_uart_debug("[*]File open failed",TRUE);
    free((void*)target_file);
    return errMsg;
  }
  *target = target_file;
  return 0;
}

int vfs_close(int fid) {
  // 1. release the file handle
  // 2. Return error code if fails
  struct file* f = get_current()->fd_table[fid];
  return f->f_ops->close(f);
}

int vfs_write(struct file* file, const void* buf, size_t len) {
  // 1. write len byte from buf to the opened file.
  // 2. return written size or error code if an error occurs.
  return file->f_ops->write(file,buf,len);
}

int vfs_read(struct file* file, void* buf, size_t len) {
  // 1. read min(len, readable size) byte to buf from the opened file.
  // 2. block if nothing to read for FIFO type
  // 2. return read size or error code if an error occurs.
  return file->f_ops->read(file,buf,len);
}

int vfs_mkdir(const char* pathname)
{
  // 1. Lookup pathname
  struct file* target_file = my_malloc(sizeof(struct file));
  struct vnode *v_node;
  // struct vnode *v_dir,*v_node;
  // v_dir = rootfs->root;
  // int ret = rootfs->root->v_ops->lookup(v_dir,&v_node,pathname);
  int ret = vfs_lookup(pathname,&v_node);
  // v_node = target_file->vnode;
  // 2. Create a new file handle for this vnode if found.
  if(ret == sucessMsg)
  {
    target_file->f_pos = 0;
    target_file->f_ops = v_node->f_ops;
    target_file->vnode = v_node;
    writes_uart_debug("[*]Directory is already existed: ",FALSE);
    writes_uart_debug((char*)pathname,TRUE);
  }
  else if(ret == lastCompNotFound){
    
    char buf[COMP_NAME_LEN];
    int i,j;
    for(i=0,j=0;i<strlen((char*)pathname);i++) // buf = component name
    {
      if(pathname[i]=='/'){
        buf[0]='\0';
        j=0;
      }else{
        buf[j++] = pathname[i];
      }
    }
    buf[j] = '\0';
    writes_uart_debug("[*]Start create dir in dir: ",FALSE);
    writes_uart_debug(((struct tmpfs_inode*)(v_node->internal))->name,TRUE);
    struct vnode *new_vnode;
    
    v_node->v_ops->mkdir(v_node,&new_vnode,buf);
    // ((struct tmpfs_inode*)(new_vnode->internal))->type = dir_n;
    target_file->f_pos = 0;
    target_file->f_ops = new_vnode->f_ops;
    target_file->vnode = new_vnode;
    writes_uart_debug("[*]Dir: ",FALSE);
    writes_uart_debug(buf,FALSE);
    writes_uart_debug(" has been created",TRUE);
  }
  else{
    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails
    writes_uart_debug("[*]Directory create failed",TRUE);
    free((void*)target_file);
    return errMsg;
  }
  // *target = target_file;
  return 0;
}

int vfs_lookup(const char* pathname, struct vnode** target) {
  struct vnode* vnode_itr = rootfs->root;
  
  char comp_name[COMP_NAME_LEN];
  int i=0,j=0;
  while(pathname[i]=='/')
  {
    i++;
    j++;
  }
  for(; i<=strlen((char*)pathname);i++)
  {
    if(pathname[i]=='/' || i == strlen((char*)pathname))
    {
      struct vnode* next_vnode;
      strncpy(comp_name,(char*)(pathname+j),i-j);
      comp_name[i-j+1]='\0';
      writes_uart_debug("[*]Look up component: ",FALSE);
      writes_uart_debug(comp_name,FALSE);
      writes_uart_debug(" in directory ",FALSE);
      writes_uart_debug(((struct tmpfs_inode*)(vnode_itr->internal))->name,TRUE);
      int ret = rootfs->root->v_ops->lookup(vnode_itr,&next_vnode,comp_name);
      if(ret != 0)
      {
        if(i==strlen((char*)pathname)){
          *target = vnode_itr;
          return lastCompNotFound;
        }
        else return ret;
      }
      j = i + 1;
      vnode_itr = next_vnode;
    }
  }
  *target = vnode_itr;
  // for (component_name : pathname) {
  //   struct node* next_vnode;
  //   int ret = vnode_itr->v_ops->lookup(vnode_itr, next_vnode, component_name);
  //   if(ret != 0) {
  //     return ret;
  //   }
  //   vnode_itr = next_vnode;
  // }
  // *target = vnode_itr;
  return 0;
}

struct vnode* vnode_create(struct vnode* dir_vnode,struct mount* mount_point,struct vnode_operations* v_ops,struct file_operations* f_ops,int n_type)
{
  struct vnode* v_node = my_malloc(sizeof(struct vnode));
  v_node->mount = mount_point;
  v_node->v_ops = v_ops;
  v_node->f_ops = f_ops;
  if(strcmp(mount_point->fs->name,"tmpfs")==0) // if the mounted fs is tmpfs.
  {
    struct tmpfs_inode* inode = my_malloc(sizeof(struct tmpfs_inode));
    inode->type = n_type;
    inode->next_sibling = nullptr;
    inode->child = nullptr;
    if(dir_vnode!=nullptr) inode->parent = dir_vnode->internal;
    else inode->parent = nullptr;
    strcpy(inode->name,"/");
    if(n_type == file_n)
    {
      inode->data = my_malloc(sizeof(struct tmpfs_block));
      strcpy(inode->data->content,"\0");
      inode->data->next = nullptr;
    }
    v_node->internal = inode;
  }
  else{
    writes_uart_debug("[*]Unidentified file system.",TRUE);
  }
  return v_node;
}

void vfs_ls()
{
  rootfs->root->v_ops->ls(get_current()->pwd);
  return;
}

int vfs_mount(const char* target, const char* filesystem)
{
  struct vnode* mount_vnode;
  struct mount* mount_point;
  int res = vfs_lookup(target,&mount_vnode);
  if(res == errMsg || ((struct tmpfs_inode*)(mount_vnode->internal))->type!=dir_n) return -1; // return err code
  if(res == compNotFound) return -2; // not found

  struct filesystem *fs = my_malloc(sizeof(struct filesystem));
  register_filesystem(fs);
  strcpy(fs->name,filesystem);
  fs->setup_mount = tmpfs_setup_mount;
  mount_point = my_malloc(sizeof(struct mount));
  res = fs->setup_mount(fs,mount_point);
  
  mount_vnode->mount = mount_point;

  return -1;
}
