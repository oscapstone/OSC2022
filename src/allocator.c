#include "allocator.h"
#include "stdlib.h"
#include "mini_uart.h"
#include "cpio.h"
#ifndef __DEBUG_LOG
#define __DEBUG_LOG
#endif
int chunk_size_arr[] = {
                        // 0x1, //2^0
                        // 0x2, //2^1
                        0x4, //2^2
                        0x8, //2^3
                        0x10, //2^4
                        0x20, //2^5
                        0x40, //2^6
                        0x80, //2^7
                        0x100, //2^8
                        0x200, //2^9
                        0x400, //2^10
                        0x800 //2^11
                        };
int chunk_arr_len = 10;
_frame_node* frame_freelist[MAX_CONTIBLOCK_SIZE+1];
_frame_chunk_node* chunk_freelist[10];

int frame_array[FRAME_ARRAY_SIZE] = {_F};
// _frame_array_node* dsdads[FRAME_ARRAY_SIZE];
void init_frame_freelist(){
    for (int i = 0; i <= MAX_CONTIBLOCK_SIZE; i++)
    {
        frame_freelist[i]=nullptr;
    }
    _F_frame_array(0,MAX_CONTIBLOCK_SIZE);
    frame_freelist[MAX_CONTIBLOCK_SIZE] = simple_malloc(sizeof(struct _frame_node));
    frame_freelist[MAX_CONTIBLOCK_SIZE]->next = nullptr;
    frame_freelist[MAX_CONTIBLOCK_SIZE]->_block_exponent = MAX_CONTIBLOCK_SIZE;
    frame_freelist[MAX_CONTIBLOCK_SIZE]->_index = 0;

    for (int i = 0; i < chunk_arr_len; i++)
    {
        chunk_freelist[i] = nullptr;
    }

    
}
void frame_list_addtail(int i,_frame_node *element){
    _frame_node* node = frame_freelist[i];
    if(node==nullptr){
        frame_freelist[i]=element;
    }
    else{
        while(node->next!=nullptr){
            node = node->next;
        }
        node->next = element;
    }
}
_frame_node* create_frame_node(int idx,int exp,_frame_node* next){
    _frame_node* new_node = simple_malloc(sizeof(_frame_node));
    new_node->next = next;
    new_node->_block_exponent = exp;
    new_node->_index=idx;
    return new_node;
}

void _F_frame_array(int idx, int exp){
    frame_array[idx] = power(2,exp)*BLOCK_SIZE;
    for (int i = idx + 1; i < idx+power(2,exp); i++)
    {
        frame_array[i] = _F;
    }

}

void _X_frame_array(int idx, int exp){
    frame_array[idx] = -power(2,exp)*BLOCK_SIZE;
    for (int i = idx+1; i < idx + power(2,exp); i++)
    {
        frame_array[i] = _X;
    }
}

void* get_freeframe_addr(unsigned int size){
    int i;
    for (i = 0; i <= MAX_CONTIBLOCK_SIZE; i++)
    {
        if(frame_freelist[i]!=nullptr && BLOCK_SIZE*power(2,i)>=size){
            writes_uart("Require ");
            write_int_uart(size,FALSE);
            // writes_uart(", From i to start:");
            // write_int_uart(i,FALSE);
            writes_uart(" ,Min free block size: ");
            write_int_uart(BLOCK_SIZE *power(2,i),TRUE);
            break;
        }
    }
    if(frame_freelist[i]==nullptr)
    {
        writes_uart("ALL FULL\r\n");
        return null;
    }
    _frame_node* node; //= frame_freelist[i];
    // frame_freelist[i] = frame_freelist[i]->next;
    while(BLOCK_SIZE*power(2,i)/2>=size && i>0){
        node = frame_freelist[i];
        writes_uart("From ");
        write_int_uart(node->_index,FALSE);
        writes_uart(" - ");
        write_int_uart(node->_index+frame_array[node->_index]/BLOCK_SIZE-1,FALSE);
        frame_freelist[i] = frame_freelist[i]->next;
        i-=1;
        _frame_node* new_node1 = create_frame_node(node->_index,i,nullptr);
        // frame_array[new_node1->_index] = new_node1->_block_exponent;
        _F_frame_array(new_node1->_index,new_node1->_block_exponent);
        frame_list_addtail(i,new_node1);
        _frame_node* new_node2 = create_frame_node(node->_index+power(2,i),i,nullptr);
        // frame_array[new_node2->_index] = new_node2->_block_exponent;
        _F_frame_array(new_node2->_index,new_node2->_block_exponent);
        frame_list_addtail(i,new_node2);
        
        writes_uart(" To ");
        write_int_uart(new_node1->_index,FALSE);
        writes_uart(" - ");
        write_int_uart(new_node1->_index+frame_array[new_node1->_index]/BLOCK_SIZE-1,FALSE);
        writes_uart(" And ");
        write_int_uart(new_node2->_index,FALSE);
        writes_uart(" - ");
        write_int_uart(new_node2->_index+frame_array[new_node2->_index]/BLOCK_SIZE-1,TRUE);
    }
    node = frame_freelist[i];
    _X_frame_array(node->_index,node->_block_exponent);
    // for (int i = node->_index; i < node->_index + power(2,node->_block_exponent); i++)
    // {
    //     frame_array[i] = _X;
    // }
    // write_int_uart(node->_index,TRUE);
    // write_int_uart(-frame_array[node->_index],TRUE);
    frame_freelist[i] = frame_freelist[i]->next;
    // write_int_uart(node->_index,TRUE);
    // write_int_uart(node->_block_exponent,TRUE);
    writes_uart("Got block size: ");
    write_int_uart(-frame_array[node->_index],TRUE);
    unsigned long long freeframe_addr = ALLOCATOR_START + node->_index*BLOCK_SIZE;
    return (void*)freeframe_addr;
    
}

int find_buddy(int frame_idx, int frame_exp){
    if(frame_exp==16)
        return frame_idx;
    int buddy_frame_idx = frame_idx^(power(2,frame_exp));
    return buddy_frame_idx;
}
void merge_buddy(int frame_idx, int buddy_idx){
    if(frame_idx == buddy_idx || frame_array[frame_idx]<BLOCK_SIZE
                              || frame_array[buddy_idx]<BLOCK_SIZE
                              || frame_array[frame_idx]>=power(2,MAX_CONTIBLOCK_SIZE)
                              || frame_array[buddy_idx]>=power(2,MAX_CONTIBLOCK_SIZE))
    {
        // write_int_uart(frame_idx,TRUE);
        // write_int_uart(frame_array[frame_idx]/BLOCK_SIZE,TRUE);
        // write_int_uart(buddy_idx,TRUE);
        // write_int_uart(frame_array[buddy_idx]/BLOCK_SIZE,TRUE);
        writes_uart("[*] No buddy can be merged. \r\n");
        return;
    }
    else{
        
        // write_int_uart(frame_array[frame_idx]<0,FALSE);
        // writes_uart(" | ");
        // write_int_uart(frame_array[buddy_idx]<0,FALSE);
        // writes_uart(" | ");
        // write_int_uart((-frame_array[frame_idx])>0,FALSE);
        // writes_uart(" | ");
        // write_int_uart((-frame_array[buddy_idx])>0,FALSE);
        // writes_uart(" | ");
        // write_int_uart(frame_array[frame_idx],FALSE);
        // writes_uart(" | ");
        // write_int_uart(frame_array[buddy_idx],FALSE);
        // writes_uart(" | ");
        // write_int_uart(-frame_array[frame_idx],FALSE);
        // writes_uart(" | ");
        // write_int_uart(-frame_array[buddy_idx],TRUE);


        int smaller_idx_frame = (frame_idx < buddy_idx)?frame_idx : buddy_idx;
        int larger_idx_frame = (smaller_idx_frame == frame_idx)?buddy_idx : frame_idx;
        writes_uart("From ");
        write_int_uart(smaller_idx_frame,FALSE);
        writes_uart(" - ");
        write_int_uart(smaller_idx_frame+frame_array[smaller_idx_frame]/BLOCK_SIZE-1,FALSE);
        writes_uart(" And ");
        write_int_uart(larger_idx_frame,FALSE);
        writes_uart(" - ");
        write_int_uart(larger_idx_frame+frame_array[larger_idx_frame]/BLOCK_SIZE-1,FALSE);

        frame_array[smaller_idx_frame]+=frame_array[larger_idx_frame];
        frame_array[larger_idx_frame] = _F;
        writes_uart(" To ");
        write_int_uart(smaller_idx_frame,FALSE);
        writes_uart(" - ");
        write_int_uart(smaller_idx_frame+frame_array[smaller_idx_frame]/BLOCK_SIZE-1,TRUE);
        int new_buddy_idx = find_buddy(smaller_idx_frame,powOf2ToExponent(frame_array[smaller_idx_frame]/BLOCK_SIZE));
        writes_uart("[*] merging ");
        write_int_uart(smaller_idx_frame,FALSE);
        writes_uart(" and ");
        write_int_uart(new_buddy_idx,TRUE);
        merge_buddy(smaller_idx_frame,new_buddy_idx);
    }
}
void free(void* frame_addr){
    //writehex_uart(frame_addr-ALLOCATOR_START,TRUE);
    int frame_idx = frameAddrToIdx(frame_addr);//(frame_addr-ALLOCATOR_START)/BLOCK_SIZE;
    if(frame_array[frame_idx]>=_X)
        return;
    else if( -frame_array[frame_idx] <= chunk_size_arr[chunk_arr_len-1]){
        chunkNode_append(frame_addr);
    }
    else{
        int frame_exp = powOf2ToExponent((-frame_array[frame_idx])/BLOCK_SIZE);
        // write_int_uart(frame_idx,TRUE);
        // write_int_uart(frame_exp,TRUE);
        _F_frame_array(frame_idx,frame_exp);
        // write_int_uart(frame_idx,TRUE);
        // write_int_uart(frame_exp,TRUE);
        int buddy_frame_idx = find_buddy(frame_idx,frame_exp);
        // write_int_uart(frame_idx,TRUE);
        // write_int_uart(buddy_frame_idx,TRUE);
        merge_buddy(frame_idx,buddy_frame_idx);
        while(frame_array[frame_idx]==_F)
            frame_idx--;
        frame_exp = powOf2ToExponent(frame_array[frame_idx]/BLOCK_SIZE);
        _frame_node* new_node = create_frame_node(frame_idx,frame_exp,nullptr);
        frame_list_addtail(frame_exp,new_node);
    }
    
}

unsigned int frameAddrToIdx(void* frame_addr){
    if(frame_addr < ALLOCATOR_START)
    {
        writes_uart("Address not in the range of frame array\r\n");
        return null;
    }

    return (unsigned int)((unsigned long long)frame_addr-ALLOCATOR_START)/BLOCK_SIZE;
}

int getBestChunkSize(int require_size){
    for (int i = 0 ; i <chunk_arr_len; i++)
    {
        if(require_size <= chunk_size_arr[i]){
            return chunk_size_arr[i];
        }
    }
    return chunk_size_arr[chunk_arr_len-1];
    
}
void chunkNode_append(void* addr){
    unsigned int frame_idx = frameAddrToIdx(addr);
    int chunk_size = -frame_array[frame_idx];
    int chunk_exp = powOf2ToExponent(chunk_size);
    _frame_chunk_node** headChunkNode = &chunk_freelist[chunk_exp-2];
    _frame_chunk_node* newChunkNode = simple_malloc(sizeof(_frame_chunk_node));
    newChunkNode->addr = addr; newChunkNode->next = null;
    while(*headChunkNode)
        headChunkNode = &((*headChunkNode)->next);
    *headChunkNode = newChunkNode;
}
void list_freeChunkNode()
{
    for (int i = 0; i < chunk_arr_len; i++)
    {
        writes_uart("[>] Chunk ");
        write_int_uart(chunk_size_arr[i],FALSE);
        if(i<10)
            writes_uart(" ");
        writes_uart(" ");
        _frame_chunk_node *node = chunk_freelist[i];
        int count=0;
        while(node != nullptr){
            //writes_uart("[");
            count++;
            // writehex_uart((unsigned long long)node->addr,FALSE);
            // writes_uart("]");
            node = node->next;
        }
        write_int_uart(count,TRUE);
        //writes_uart("\r\n");
    }
    
}
void* get_freechunk_addr(int size){
    //int chunk_size = getBestChunkSize(size);
    int chunk_exp = powOf2ToExponent(size);
    if(chunk_freelist[chunk_exp-2]==nullptr){
        // if no free chunk exist, cut a frame into chunks and return first chunk.
        void* freeframe_addr = get_freeframe_addr(BLOCK_SIZE);
        int frame_idx = frameAddrToIdx(freeframe_addr);
        frame_array[frame_idx] = -size;
        int chunk_count = BLOCK_SIZE / size;
        
        for (int i = 1; i < chunk_count; i++)
        {
            // _frame_chunk_node* newChunkNode= simple_malloc(sizeof(_frame_chunk_node));
            // newChunkNode->addr = freeframe_addr + i*size;
            chunkNode_append(freeframe_addr + i*size);
            // headChunkNode->next = newChunkNode;
            // headChunkNode = headChunkNode->next;
        }
        // headChunkNode->next = nullptr;
        return freeframe_addr;
    }
    else{
        void* chunk_addr = chunk_freelist[chunk_exp-2]->addr;
        chunk_freelist[chunk_exp-2] = chunk_freelist[chunk_exp-2]->next;
        return chunk_addr; 
    }
    return nullptr;
}

void* my_malloc(unsigned int size){
    void* malloc_addr = nullptr;
    if(size<=chunk_size_arr[chunk_arr_len-1]){
        writes_uart("Chunk malloc ");

        int chunk_size = getBestChunkSize(size);
        write_int_uart(chunk_size,TRUE);
        malloc_addr = get_freechunk_addr(chunk_size);
        
        // unsigned int chunk_frame_idx = frameAddrToIdx(malloc_addr);
        // frame_array[chunk_frame_idx] = chunk_size;
    }
    else{
        malloc_addr = get_freeframe_addr(size);
    }
    return malloc_addr;
}

void memory_reserve(unsigned long long start, unsigned long long end){
    unsigned int start_idx = frameAddrToIdx((void*)start);
    unsigned int end_idx = frameAddrToIdx((void*)end);
    for (unsigned int i = start_idx; i <= end_idx; i++)
    {
        frame_array[i] = _R;
    }
    
}
void free_unreserved_memory(){
    for (unsigned long long i = 0; i < FRAME_ARRAY_SIZE; i++)
    {
        if(frame_array[i]!=_R)
            free((void*)(ALLOCATOR_START+BLOCK_SIZE*i));
    }
    
}
void init_memory(){
    writes_uart("Initializing memory...\r\n");
    //init_frame_freelist();
    for (int i = 0; i < FRAME_ARRAY_SIZE; i++) frame_array[i] = _X;
    
    memory_reserve(0,0x1000); //Spin tables for multicore boot
    memory_reserve(0x80000,(unsigned long long)get_smalloc_end());
    memory_reserve((unsigned long long)fdt_traverse(initramfs_start_callback),(unsigned long long)fdt_traverse(initramfs_end_callback));
    free_unreserved_memory();
}