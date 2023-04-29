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

FILE * debug;
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
        unsigned int lba : 16; // page
        unsigned int nand: 16; // block
    } fields;
};

PCA_RULE curr_pca;
static unsigned int get_next_pca();

unsigned int* L2P,* P2L,* valid_count, free_block_number;

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
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, my_pca.fields.nand); // parse block

    //write
    if ( (fptr = fopen(nand_name, "r+")))
    {
        fseek( fptr, my_pca.fields.lba * 512, SEEK_SET ); // parse page
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
    free_block_number++;
    return 1;
}


void print_pca()
{
    fprintf(debug,"[print_pca]\n");
    for(int lba = 0 ; lba < PAGE_PER_BLOCK ; lba ++){
        for(int nand = 0; nand < PHYSICAL_NAND_NUM ; nand++){

            if(P2L[nand*10+lba] == STALE_PCA)
                fprintf(debug,"[%d]=stale\t",nand * 10 + lba);
            else if(P2L[nand*10+lba] == INVALID_PCA)
                fprintf(debug,"[%d]=free\t",nand * 10 + lba);
            else
                fprintf(debug,"[%d]=used\t",nand * 10 + lba);

            if(nand*10+lba == curr_pca.fields.nand * 10 + curr_pca.fields.lba)
                fprintf(debug, "*");
        }
        fprintf(debug,"\n");
    }
    //fflush(debug);
}

int find_empty_block()
{
    int empty_page_count = 0;

    for(int nand = 0 ; nand < PHYSICAL_NAND_NUM; nand ++){
        empty_page_count = 0;
        for(int lba = 0; lba < PAGE_PER_BLOCK; lba++){
            if(P2L[nand*10+lba] == STALE_PCA)
                empty_page_count++;
        }
        if(empty_page_count==10){
            fprintf(debug,"find nand_%d is empty block\n",nand);
            //fflush(debug);
            return nand;
        }
    }
    return -1;
}

int try_free_a_block()
{
    int free_block_nand = find_empty_block();
    if(free_block_nand != -1){
        for(int lba = 0; lba < PAGE_PER_BLOCK; lba++)
        P2L[free_block_nand*10 + lba] = INVALID_PCA;
        nand_erase(free_block_nand);
        print_pca();
        return 1;
    }
    return 0;
}

static unsigned int get_next_block()
{
    // printf("[get_next_block]\n");
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
    // printf("[get_next_pca]\n");
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
            fprintf(debug, "=======OUT_OF_BLOCK=======\n");
            //fflush(debug);
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
    unsigned int pca = L2P[lba];
    if(pca != INVALID_PCA)
        nand_read(buf,pca);
    
    return 0;
}

void do_GC()
{
    fprintf(debug, "=======GC=======\n");
    char * mov_buf = calloc(512, sizeof(char));
    if(!try_free_a_block()){ // can't direct release a block, move vaild to new pca until free a block

        // find block that have least valid
        int min_valid_count = 100, target_nand = -1;
        for(int nand = 0 ; nand < PHYSICAL_NAND_NUM ; nand ++){ 
            int valid_count = 0;
            for(int lba = 0; lba < PAGE_PER_BLOCK ; lba++){
                if(P2L[nand*10+lba] != STALE_PCA && P2L[nand*10+lba] != INVALID_PCA)
                    valid_count ++;
            }
            if(min_valid_count > valid_count && valid_count > 0 && nand != curr_pca.fields.nand) {
                min_valid_count = valid_count;
                target_nand = nand;
            }
        }
        fprintf(debug, "target_nand = %d\n",target_nand);
        print_pca();
        for(int lba = 0; lba < PAGE_PER_BLOCK ; lba++){
            if(P2L[target_nand*10+lba] != STALE_PCA && P2L[target_nand*10+lba] != INVALID_PCA ){ // vaild
                PCA_RULE mov_pca, valid_pca;
                // get valid_pca
                int lba_idx = P2L[target_nand*10+lba];
                valid_pca.pca = L2P[lba_idx];
                fprintf(debug, "lba_idx = %d is valid\n",lba_idx);
                // move to new pca
                mov_pca.pca = get_next_pca();
                nand_read(mov_buf,valid_pca.pca);
                nand_write(mov_buf,mov_pca.pca);
                L2P[lba_idx] = mov_pca.pca; // update L2P
                P2L[mov_pca.fields.nand * 10 + mov_pca.fields.lba] = lba_idx; // update P2L

                P2L[target_nand*10+lba] = STALE_PCA; // make stale
            }
        }
        print_pca();
        try_free_a_block();
    }
}

static int ftl_write(const char* buf,size_t lba_range, size_t lba)
{
    //TODO
    int idx,pca_idx;

    for(idx = 0 ; idx <lba_range ; idx++)
    {
        //GC
        if(free_block_number<=0){

            // find block that have least valid
            int min_valid_count = 100;
            for(int nand = 0 ; nand < PHYSICAL_NAND_NUM ; nand ++){ 
                int valid_count = 0;
                for(int lba = 0; lba < PAGE_PER_BLOCK ; lba++){
                    if(P2L[nand*10+lba] != STALE_PCA && P2L[nand*10+lba] != INVALID_PCA)
                        valid_count ++;
                }
                if(min_valid_count > valid_count && nand != curr_pca.fields.nand) {
                    min_valid_count = valid_count;
                }
            }
            // print_pca();
            int free_page_count = PAGE_PER_BLOCK - curr_pca.fields.lba;

            fprintf(debug, "free_page_count =%d\n",free_page_count);
            fprintf(debug, "min_valid_count =%d\n",min_valid_count);
            if(free_page_count == min_valid_count + 1)
                do_GC();
        }

        if(L2P[lba+idx] != INVALID_PCA){// Find stale page
            PCA_RULE stale_pca;
            // fprintf(debug, "[stale]lba_idx = %ld\n",lba+idx);
            // fflush(debug);
            stale_pca.pca = L2P[lba+idx];    
            P2L[stale_pca.fields.nand * 10 + stale_pca.fields.lba] = STALE_PCA;
        }

        PCA_RULE cur_pca;
        cur_pca.pca = get_next_pca();// allocate new pca 
        pca_idx = cur_pca.fields.nand * 10 + cur_pca.fields.lba;
        // fprintf(debug,"pca_idx = %d\n",pca_idx);
        // fflush(debug);
        L2P[lba+idx] = cur_pca.pca; // update L2P
        P2L[pca_idx] = lba+idx; // update P2L
        nand_write(buf+idx*512,cur_pca.pca); // call nand_write
    }
    return 0;
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
    // printf("[ssd_getattr]\n");
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
    // printf("[ssd_open]\n");
    (void) fi;
    if (ssd_file_type(path) != SSD_NONE)
    {
        return 0;
    }
    return -ENOENT;
}

static int ssd_do_read(char* buf, size_t size, off_t offset)
{
    int tmp_lba, tmp_lba_range;
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
        ftl_read(tmp_buf+i*512,tmp_lba+i);
    }

    memcpy(buf, tmp_buf + offset % 512, size);

    
    free(tmp_buf);
    return size;
}
static int ssd_read(const char* path, char* buf, size_t size,
                    off_t offset, struct fuse_file_info* fi)
{
    printf("[ssd_read]\n");
    (void) fi;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
    return ssd_do_read(buf, size, offset);
}
static int ssd_do_write(const char* buf, size_t size, off_t offset)
{
    // fprintf(debug, "\nsize = %ld, offset = %ld, strlen(buf) = %ld\n", size, offset, strlen(buf));
    // fflush(debug);
    int tmp_lba, tmp_lba_range, tmp_offset, idx;
    // int process_size, curr_size, remain_size, rst;
    char* tmp_buf;

    host_write_size += size;
    if (ssd_expand(offset + size) != 0)
    {
        return -ENOMEM;
    }

    tmp_lba = offset / 512; // start lba
    tmp_offset = offset % 512; // current offset
    tmp_lba_range = (offset + size - 1) / 512 - (tmp_lba) + 1; // will use range
    tmp_buf = calloc(tmp_lba_range * 512, sizeof(char));

    for (idx = 0; idx < tmp_lba_range; idx++)
    {
        ftl_read(tmp_buf+idx*512,tmp_lba+idx);
    }

    memcpy(tmp_buf+tmp_offset, buf, size);
    ftl_write(tmp_buf, tmp_lba_range, tmp_lba);
    return size;
}
static int ssd_write(const char* path, const char* buf, size_t size,
                     off_t offset, struct fuse_file_info* fi)
{
    printf("[ssd_write]\n");
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
    printf("[ssd_truncate]\n");   
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
    printf("[ssd_readdir]\n");
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
    // printf("[ssd_ioctl]\n");
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

    debug = fopen ("log.txt", "w+");
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
