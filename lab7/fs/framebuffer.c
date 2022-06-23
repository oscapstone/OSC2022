#include "fs/framebuffer.h"

struct framebuffer_info fb_info;

struct filesystem_type framebufferfs = {
    .fs_name = "framebufferfs",
    .mount = framebufferfs_mount
};

struct inode_operations framebufferfs_i_ops = {
	.create = framebufferfs_create,
    .lookup = framebufferfs_lookup,
    .mkdir = framebufferfs_mkdir
};

struct file_operations framebufferfs_f_ops = {
    .lseek64 = framebufferfs_lseek64,
    .read =  framebufferfs_read,
    .write = framebufferfs_write,
    .open = framebufferfs_open,
    .flush = framebufferfs_flush,
    .release = framebufferfs_release,
    .ioctl = framebufferfs_ioctl
};

struct dentry* framebufferfs_create(struct dentry * parent, const char* new_file_name){
    FS_LOG("framebufferfs_create");
    return NULL;
}

struct dentry *framebufferfs_lookup(struct dentry *parent, char* target){
    FS_LOG("framebufferfs_lookup");
    return NULL;
}

int framebufferfs_lseek64(struct file * file, loff_t offset, int whence){
    FS_LOG("framebufferfs_lseek64");
    loff_t new_offset;
    switch (whence) {
        case SEEK_SET:
            new_offset = offset;
            break;
        case SEEK_CUR:
            new_offset = file->f_pos + offset;
            break;
        case SEEK_END:
            new_offset = fb_info.lfb_size;
            break;
        default:
            return -1;
    }
    if(new_offset > fb_info.lfb_size) return -1;
    file->f_pos = new_offset;
    return 0;
}

long framebufferfs_read(struct file *file, char *buf, size_t len, loff_t *offset){
    return -1;
}

long framebufferfs_write(struct file * file, char * buf, size_t len, loff_t * offset){
    uint64_t daif = local_irq_disable_save();
    struct dentry* d_file = file->f_dentry;
    struct tmpfs_file* tmp_file = d_file->d_inode->private_data;
    if(*offset + len > fb_info.lfb_size) len = fb_info.lfb_size - *offset;

    memcpy((void*)(fb_info.lfb + *offset), buf, len);
    *offset += len;

    local_irq_restore(daif);

    return len;
}

struct file* framebufferfs_open(struct dentry* d_file,uint32_t flags,umode_t mode){
    FS_LOG("framebufferfs_open");
    return create_file(d_file, flags, mode); 
}
int framebufferfs_flush(struct file *){
    FS_LOG("framebufferfs_flush");
    return 0;
}
int framebufferfs_release(struct inode * inode, struct file * file){
    FS_LOG("framebufferfs_release");
    if(file == NULL) return -1;

    file->f_count--;
    if(file->f_count == 0) kfree(file); 
    return 0;
}

struct mount* framebufferfs_mount(struct filesystem_type* fs_type, struct dentry* target){
    FS_LOG("framebufferfs_mount");
    uint64_t daif = local_irq_disable_save();
    struct dentry* new_root, *link; 
    new_root->d_parent = target->d_parent;
    struct inode* root_node = create_inode(&framebufferfs_f_ops, &framebufferfs_i_ops, S_IFCHR);
    struct mount* ret = (struct mount*)kmalloc(sizeof(struct mount));

    new_root = create_dentry("framebufferfs", 0, root_node);
    target->d_mnt = ret;
    ret->mnt_root = new_root;
    local_irq_restore(daif);
    return ret;
}

int framebufferfs_ioctl(struct file*file, unsigned long request, va_list){
    unsigned int __attribute__((aligned(16))) mbox[36];
    unsigned int width, height, pitch, isrgb; /* dimensions and channel order */
    unsigned char *lfb;                       /* raw frame buffer address */

    mbox[0] = 35 * 4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003; // set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024; // FrameBufferInfo.width
    mbox[6] = 768;  // FrameBufferInfo.height

    mbox[7] = 0x48004; // set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024; // FrameBufferInfo.virtual_width
    mbox[11] = 768;  // FrameBufferInfo.virtual_height

    mbox[12] = 0x48009; // set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // FrameBufferInfo.x_offset
    mbox[16] = 0; // FrameBufferInfo.y.offset

    mbox[17] = 0x48005; // set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // FrameBufferInfo.depth

    mbox[21] = 0x48006; // set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB, not BGR preferably

    mbox[25] = 0x40001; // get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // FrameBufferInfo.pointer
    mbox[29] = 0;    // FrameBufferInfo.size

    mbox[30] = 0x40008; // get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // FrameBufferInfo.pitch

    mbox[34] = MBOX_TAG_LAST;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    if (Mbox_call(mbox, MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0) {
        mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
        fb_info.width = mbox[5];        // get actual physical width
        fb_info.height = mbox[6];       // get actual physical height
        fb_info.pitch = mbox[33];       // get number of bytes per line
        fb_info.isrgb = mbox[24];       // get the actual channel order
        fb_info.lfb = mbox[28];
        fb_info.lfb_size = mbox[29];
        FS_LOG("lfb: %p", fb_info.lfb);
    }
    return 0;
}

int framebufferfs_mkdir(struct dentry * parent, const char * target, umode_t mode){
    return -1;
}
