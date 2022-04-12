#include <malloc.h>
#include <uart.h>
#include <string.h>
#include <allocator.h>

/* define 10 level common chunk size */
unsigned int chunk_size[] = {0x10, 0x20, 0x40, 0x60, 0xa0, 0xe0, 0x100, 0x200, 0x400, 0x800};
FreeChuckList freechunk_list[MAX_CHUNK_SIZE];

int find_level(unsigned int size){
    for(int i = 0; i < chunk_size; i++){
        if(size <= chunk_size[i]){
            return i;
        }
    }
    return -1;
}
void freechunk_list_init(){
    for(int i = 0; i < MAX_CHUNK_SIZE; i++){
        INIT_LIST_HEAD(&freechunk_list[i].list);
    }
}

void *chunk_alloc(unsigned int size){
    int level = -1;
    if(size > FRAME_SIZE) return NULL;
    if((level = find_level) == -1) return NULL;
    

    if(list_empty(&freechunk_list[level].list)){
        // [TODO] alloc new frame ->  split the level size
    }


    return NULL;
}


void *simple_malloc(unsigned long size) {
    static void *head = (void *)MALLOC_BASE;
    void *ptr = head;
    head = head + size;
    return ptr;
}