#include "lib/cpio.h"

uint32_t cpio_hex2dec(const char *s){
    uint32_t ret = 0;
    for(uint32_t i = 0 ; i < 8 ; i++){
        ret = ret << 4;
        ret = ret + hex2dec(s[i]);
    }
    return ret;
}

void cpio_iter_parse(struct cpio_iter* iter, struct fentry* f){
    LOG("Enter cpio_iter_parse");
    size_t count = 0;
    struct cpio_newc_header* header = (struct cpio_newc_header* )iter->cur;
    
    if(!memcpy(header->c_magic, "070701", 6)){
        LOG("CPIO PARSE ERROR!!!!");
        return;
    }
    
    f->ino = cpio_hex2dec(header->c_ino);
    f->mode = cpio_hex2dec(header->c_mode);
    f->uid = cpio_hex2dec(header->c_uid);
    LOG("f->uid: %u", f->uid);
    f->gid = cpio_hex2dec(header->c_gid);
    f->nlink = cpio_hex2dec(header->c_nlink);
    f->mtime = cpio_hex2dec(header->c_mtime);
    f->filesize = cpio_hex2dec(header->c_filesize);
    LOG("f->filesize: %u", f->filesize);
    f->devmajor = cpio_hex2dec(header->c_devmajor);
    f->devminor = cpio_hex2dec(header->c_devminor);
    f->rdevmajor = cpio_hex2dec(header->c_rdevmajor);
    f->rdevminor = cpio_hex2dec(header->c_rdevminor);
    f->namesize = cpio_hex2dec(header->c_namesize);
    LOG("f->namesize: %u", f->namesize);
    f->check = cpio_hex2dec(header->c_check);
    count = count + sizeof(struct cpio_newc_header); 

    f->filename = (char*)simple_malloc(f->namesize);
    memcpy(f->filename, (uint8_t* )header + count, f->namesize);
    LOG("get f->filename");
    count = ALIGN_UP(count + f->namesize, 4);
    if(f->filesize > 0){
        f->data = (uint8_t*)simple_malloc(f->filesize);
        memcpy(f->data, (uint8_t* )header + count, f->filesize);
    }
    LOG("get f->data");
    count = ALIGN_UP(count + f->filesize, 4);
    iter->cur = iter->cur + count;
    LOG("count: %u, iter->cur: %p", count, iter->cur); 
    LOG("Leave cpio_iter_parse");
}

void cpio_iter_init(struct cpio_iter* iter, void* addr){
    iter->cur = addr;
}
int cpio_is_tailer(struct fentry* f){
    return (strcmp(f->filename, "TRAILER!!!") == 0);
}

