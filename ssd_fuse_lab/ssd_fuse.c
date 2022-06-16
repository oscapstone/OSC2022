/*
  FUSE ssd: FUSE ioctl example
  Copyright (C) 2008       SUSE Linux Products GmbH
  Copyright (C) 2008       Tejun Heo <teheo@suse.de>
  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/
#define FUSE_USE_VERSION 35
#include <fuse.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "ssd_fuse_header.h"
#define SSD_NAME       "ssd_file"
enum
{
    SSD_NONE,
    SSD_ROOT,
    SSD_FILE,
};


static size_t physic_size;
static size_t logic_size;
static size_t host_write_size;
static size_t nand_write_size;

typedef union pca_rule PCA_RULE;
union pca_rule
{
    unsigned int pca;
    struct
    {
        unsigned int lba : 16;
        unsigned int nand: 16;
    } fields;
};

PCA_RULE curr_pca;
static unsigned int get_next_pca();

unsigned int* L2P,* P2L,* valid_count, free_block_number;

void list_validcount(){
    for(int i=0;i<PHYSICAL_NAND_NUM;i++){
        printf("VAL%d:%d\n",i,valid_count[i]);
    }
}
void list_page(){
    for (int i = 0; i < PHYSICAL_NAND_NUM; i++)
    {
        printf("%d: ",i);
        for (int j = 0; j < PAGE_PER_BLOCK; j++)
        {
            size_t pca_idx = 10*i+j;
            if(P2L[pca_idx]!=INVALID_LBA){
                printf("O");
            }
            else{
                printf("X");
            }
        } 
        printf("\n");
    }
    
}
int gcable(){
    
    int cur_valc = valid_count[curr_pca.fields.nand];
    
}
static int ssd_resize(size_t new_size)
{
    //set logic size to new_size
    if (new_size > NAND_SIZE_KB * 1024)
    {
        return -ENOMEM;
    }
    else
    {
        logic_size = new_size;
        return 0;
    }

}

static int ssd_expand(size_t new_size)
{
    //logic must less logic limit

    if (new_size > logic_size)
    {
        return ssd_resize(new_size);
    }

    return 0;
}

static int nand_read(char* buf, int pca)
{
    char nand_name[100];
    FILE* fptr;

    PCA_RULE my_pca;
    my_pca.pca = pca;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, my_pca.fields.nand);

    //read
    if ( (fptr = fopen(nand_name, "r") ))
    {
        fseek( fptr, my_pca.fields.lba * 512, SEEK_SET );
        fread(buf, 1, 512, fptr);
        fclose(fptr);
    }
    else
    {
        printf("open file fail at nand read pca = %d\n", pca);
        return -EINVAL;
    }
    return 512;
}
static int nand_write(const char* buf, int pca)
{
    char nand_name[100];
    FILE* fptr;

    PCA_RULE my_pca;
    my_pca.pca = pca;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, my_pca.fields.nand);

    //write
    if ( (fptr = fopen(nand_name, "r+")))
    {
        fseek( fptr, my_pca.fields.lba * 512, SEEK_SET );
        fwrite(buf, 1, 512, fptr);
        fclose(fptr);
        physic_size ++;
        valid_count[my_pca.fields.nand]++;
    }
    else
    {
        printf("open file fail at nand (%s) write pca = %d, return %d\n", nand_name, pca, -EINVAL);
        return -EINVAL;
    }

    nand_write_size += 512;
    return 512;
}

static int nand_erase(int block_index)
{
    char nand_name[100];
    FILE* fptr;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, block_index);
    fptr = fopen(nand_name, "w");
    if (fptr == NULL)
    {
        printf("erase nand_%d fail", block_index);
        return 0;
    }
    fclose(fptr);
    valid_count[block_index] = FREE_BLOCK;
    return 1;
}
int byte_count[PHYSICAL_NAND_NUM];
static unsigned int get_next_block()
{
    for (int i = 0; i < PHYSICAL_NAND_NUM; i++)
    {
        if (valid_count[(curr_pca.fields.nand + i) % PHYSICAL_NAND_NUM] == FREE_BLOCK)
        {
            curr_pca.fields.nand = (curr_pca.fields.nand + i) % PHYSICAL_NAND_NUM;
            curr_pca.fields.lba = 0;
            free_block_number--;
            valid_count[curr_pca.fields.nand] = 0;
            return curr_pca.pca;
        }
    }
    return OUT_OF_BLOCK;
}
static unsigned int get_next_pca()
{
    if (curr_pca.pca == INVALID_PCA)
    {
        //init
        curr_pca.pca = 0;
        valid_count[0] = 0;
        free_block_number--;
        return curr_pca.pca;
    }

    if(curr_pca.fields.lba == 9)
    {
        int temp = get_next_block();
        if (temp == OUT_OF_BLOCK)
        {
            // gc?
            return OUT_OF_BLOCK;
        }
        else if(temp == -EINVAL)
        {
            return -EINVAL;
        }
        else
        {
            return temp;
        }
    }
    else
    {
        curr_pca.fields.lba += 1;
    }
    return curr_pca.pca;

}

static int ftl_read( char* buf, size_t lba)
{
    // TODO
    // 1 sector = 512 bytes

    size_t pca = L2P[lba];
    if(pca==INVALID_PCA) return 0;
    return nand_read(buf,pca);
}
static int ssd_do_read(char* buf, size_t size, off_t offset)
{
    int tmp_lba, tmp_lba_range, rst ;
    char* tmp_buf;

    //off limit
    if ((offset ) >= logic_size)
    {
        return 0;
    }
    if ( size > logic_size - offset)
    {
        //is valid data section
        size = logic_size - offset;
    }

    tmp_lba = offset / 512;
    tmp_lba_range = (offset + size - 1) / 512 - (tmp_lba) + 1;
    tmp_buf = calloc(tmp_lba_range * 512, sizeof(char));


    for (int i = 0; i < tmp_lba_range; i++) {
        
        // TODO
        if(ftl_read(tmp_buf+i*512,tmp_lba+i)==-EINVAL) return -EINVAL;
    }

    memcpy(buf, tmp_buf + offset % 512, size);

    
    free(tmp_buf);
    return size;
}
int isBlockFull(int bid){
    if(curr_pca.fields.nand!=bid && valid_count[bid]!=FREE_BLOCK) return 1;
    else if(curr_pca.fields.nand==bid && curr_pca.fields.lba==9) return 1;
    else return 0;
}
void garbage_collect()
{
    // sort the valid count array with index array
    int vc_idx[PHYSICAL_NAND_NUM];
    int vc[PHYSICAL_NAND_NUM];
    int i;
    for (i = 0; i < PHYSICAL_NAND_NUM; i++) {
        vc[i]=valid_count[i];
        vc_idx[i]=i;
    }
    for (i = 0; i < PHYSICAL_NAND_NUM; i++)
    {
        int min = __INT_MAX__;
        int min_i = -1;
        for (int j = i; j < PHYSICAL_NAND_NUM; j++)
            if(vc[j]<min){
                min = vc[j];
                min_i = j;
            }
        int tmp;
        tmp = vc[i];
        vc[i] = vc[min_i];
        vc[min_i] = tmp;
        tmp = vc_idx[i];
        vc_idx[i]=vc_idx[min_i];
        vc_idx[min_i]=tmp;
    }

    // find the first i that valid count != -1(not empty)
    for (i = 0; i < PHYSICAL_NAND_NUM; i++)if(vc[i]!=-1) break;

    if(vc[i]+vc[i+1]>PAGE_PER_BLOCK) return;
    if(vc_idx[i]==curr_pca.fields.nand && vc[i]+vc[i+1]<PAGE_PER_BLOCK) return;
 
    // if(!isBlockFull(curr_pca.fields.nand)) return;
    // if(vc_idx[i]==curr_pca.fields.nand && vc[i]!=PAGE_PER_BLOCK) return;
    
    // printf("--------------------DO GC----------------------\n");
    // for (int j = 0; j < PHYSICAL_NAND_NUM; j++)
    // {
    //     printf("VC[%d]:%d\n",vc_idx[j],vc[j]);
    // }
    
    // write buffer pages to new block
    // list_page();
    unsigned int fb = get_next_block();
    char* buf = malloc(PAGE_PER_BLOCK*512);
    size_t buf_offset = 0;
    memset(buf,INVALID_PCA,PAGE_PER_BLOCK*512);
    for (; i < PHYSICAL_NAND_NUM; i++)
    {   
        int b_idx = vc_idx[i];
        for(size_t pca=65536*b_idx;pca<65536*b_idx+PAGE_PER_BLOCK;pca++){
            if(buf_offset+512>512*PAGE_PER_BLOCK){
                goto afterCopy;
            }
            size_t pca_idx = 10*(pca/65536) + pca%65536;
            size_t lba = P2L[pca_idx];
            if(lba!=INVALID_LBA){
                
                // int lba_idx = lba/512;
                size_t new_pca = fb+buf_offset/512;
                int new_pca_idx = 10*(fb/65536)+buf_offset/512;
                ssd_do_read((char*)(buf+buf_offset),512,lba*512);
                P2L[pca_idx] = INVALID_LBA;
                nand_write(buf+buf_offset,fb+buf_offset/512);
                curr_pca.pca = fb+buf_offset/512;
                L2P[lba] = new_pca;
                P2L[new_pca_idx]=lba;
                // printf("Block %d move page%d to %d,oldpcaid:%d,newpca:%d\n",b_idx,pca_idx%10,fb/65536,pca_idx,fb+buf_offset/512);
                buf_offset+=512;
                valid_count[b_idx]--;            
            }
        }
    }
    afterCopy:
        free(buf);
        for (int i = 0; i < PHYSICAL_NAND_NUM; i++)
        {
            if(valid_count[i]==0){
                nand_erase(i);
                free_block_number++;
            }
        }
}
static int ftl_write(const char* buf, size_t lba_rnage, size_t lba)
{
    // TODO
    if(free_block_number<=1) garbage_collect();

    size_t pca;
    unsigned int pca_idx;

    // if exist data in logical block would be overwrite.
    // mark it as invalid, then write new data to new pca.
    if(L2P[lba]!=INVALID_PCA){
        pca = L2P[lba];
        pca_idx = 10*(pca/65536) + pca%65536; // ex. 65537-> 11
        P2L[pca_idx]=INVALID_LBA; // mark the phy page as invalid
        valid_count[pca_idx/10]--; //valid count of that block
    }
    pca = get_next_pca();
    pca_idx = 10*(pca/65536)+pca%65536;
    //open file failed
    if(nand_write(buf,pca)==-EINVAL) return -EINVAL;

    L2P[lba] = pca; // manage l2p table
    P2L[pca_idx] = lba; // manage p2l table
}

static int ssd_file_type(const char* path)
{
    if (strcmp(path, "/") == 0)
    {
        return SSD_ROOT;
    }
    if (strcmp(path, "/" SSD_NAME) == 0)
    {
        return SSD_FILE;
    }
    return SSD_NONE;
}
static int ssd_getattr(const char* path, struct stat* stbuf,
                       struct fuse_file_info* fi)
{
    (void) fi;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();
    stbuf->st_atime = stbuf->st_mtime = time(NULL);
    switch (ssd_file_type(path))
    {
        case SSD_ROOT:
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            break;
        case SSD_FILE:
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            stbuf->st_size = logic_size;
            break;
        case SSD_NONE:
            return -ENOENT;
    }
    return 0;
}
static int ssd_open(const char* path, struct fuse_file_info* fi)
{
    (void) fi;
    if (ssd_file_type(path) != SSD_NONE)
    {
        return 0;
    }
    return -ENOENT;
}

static int ssd_read(const char* path, char* buf, size_t size,
                    off_t offset, struct fuse_file_info* fi)
{
    (void) fi;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
    return ssd_do_read(buf, size, offset);
}
static int ssd_do_write(const char* buf, size_t size, off_t offset)
{
    int tmp_lba, tmp_lba_range, process_size;
    int idx, curr_size, remain_size, rst;
    char* tmp_buf;
    
    host_write_size += size;
    if (ssd_expand(offset + size) != 0)
    {
        return -ENOMEM;
    }
    
    tmp_lba = offset / 512;
    tmp_lba_range = (offset + size - 1) / 512 - (tmp_lba) + 1;

    process_size = 0;
    remain_size = size;
    curr_size = 0;
    tmp_buf = malloc(512);
    for (idx = 0; idx < tmp_lba_range; idx++)
    {
        memset(tmp_buf,0,512);
        if(L2P[tmp_lba+idx]!=INVALID_PCA){
            // read modify write
            int ret=ssd_do_read(tmp_buf,512,(tmp_lba+idx)*512);
        }
        if((idx==0)&&offset%512!=0){
            process_size = (remain_size>512-offset%512)?512-offset%512:remain_size;
            memcpy((char*)(tmp_buf+offset%512),buf,process_size); //??
        }
        else{
            process_size = (remain_size>512)?512:remain_size;
            memcpy(tmp_buf,(char*)(buf+curr_size),process_size); 
        }
        // printf("-----write %d to offset %d\n",size,offset);
        ftl_write(tmp_buf,tmp_lba_range,tmp_lba+idx);
        // printf("*********[%d]offset:%d,process:%d,cur:%d->%d,remain:%d->%d\n",idx,offset,process_size,curr_size,curr_size+process_size,remain_size,remain_size-process_size);
        curr_size+=process_size;
        remain_size -= process_size;
    }
    free(tmp_buf);
    return size;
}
static int ssd_write(const char* path, const char* buf, size_t size,
                     off_t offset, struct fuse_file_info* fi)
{

    (void) fi;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
    return ssd_do_write(buf, size, offset);
}
static int ssd_truncate(const char* path, off_t size,
                        struct fuse_file_info* fi)
{
    (void) fi;
    memset(L2P, INVALID_PCA, sizeof(int) * LOGICAL_NAND_NUM * PAGE_PER_BLOCK);
    memset(P2L, INVALID_LBA, sizeof(int) * PHYSICAL_NAND_NUM * PAGE_PER_BLOCK);
    memset(valid_count, FREE_BLOCK, sizeof(int) * PHYSICAL_NAND_NUM);
    curr_pca.pca = INVALID_PCA;
    free_block_number = PHYSICAL_NAND_NUM;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }

    return ssd_resize(size);
}
static int ssd_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info* fi,
                       enum fuse_readdir_flags flags)
{
    (void) fi;
    (void) offset;
    (void) flags;
    if (ssd_file_type(path) != SSD_ROOT)
    {
        return -ENOENT;
    }
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, SSD_NAME, NULL, 0, 0);
    return 0;
}
static int ssd_ioctl(const char* path, unsigned int cmd, void* arg,
                     struct fuse_file_info* fi, unsigned int flags, void* data)
{

    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
    if (flags & FUSE_IOCTL_COMPAT)
    {
        return -ENOSYS;
    }
    switch (cmd)
    {
        case SSD_GET_LOGIC_SIZE:
            *(size_t*)data = logic_size;
            return 0;
        case SSD_GET_PHYSIC_SIZE:
            *(size_t*)data = physic_size;
            return 0;
        case SSD_GET_WA:
            *(double*)data = (double)nand_write_size / (double)host_write_size;
            return 0;
    }
    return -EINVAL;
}
static const struct fuse_operations ssd_oper =
{
    .getattr        = ssd_getattr,
    .readdir        = ssd_readdir,
    .truncate       = ssd_truncate,
    .open           = ssd_open,
    .read           = ssd_read,
    .write          = ssd_write,
    .ioctl          = ssd_ioctl,
};
int main(int argc, char* argv[])
{
    int idx;
    char nand_name[100];
    physic_size = 0;
    logic_size = 0;
    curr_pca.pca = INVALID_PCA;
    free_block_number = PHYSICAL_NAND_NUM;
    
    L2P = malloc(LOGICAL_NAND_NUM * PAGE_PER_BLOCK * sizeof(int));
    memset(L2P, INVALID_PCA, sizeof(int) * LOGICAL_NAND_NUM * PAGE_PER_BLOCK);
    P2L = malloc(PHYSICAL_NAND_NUM * PAGE_PER_BLOCK * sizeof(int));
    memset(P2L, INVALID_LBA, sizeof(int) * PHYSICAL_NAND_NUM * PAGE_PER_BLOCK);
    valid_count = malloc(PHYSICAL_NAND_NUM * sizeof(int));
    memset(valid_count, FREE_BLOCK, sizeof(int) * PHYSICAL_NAND_NUM);

    //create nand file
    for (idx = 0; idx < PHYSICAL_NAND_NUM; idx++)
    {
        FILE* fptr;
        snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, idx);
        fptr = fopen(nand_name, "w");
        if (fptr == NULL)
        {
            printf("open fail");
        }
        fclose(fptr);
    }
    return fuse_main(argc, argv, &ssd_oper, NULL);
}
