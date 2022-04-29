// #include "list.h"
#ifndef __ALLCATOR_H
#define __ALLCATOR_H

#define ALLOCATOR_BASE 0x00000000
#define ALLOCATOR_START ALLOCATOR_BASE
#define ALLOCATOR_END ALLOCATOR_BASE+ 0x3C000000
#define BLOCK_SIZE 0x1000
#define MAX_CONTIBLOCK_SIZE 12

typedef struct _frame_node
{
    int _block_exponent;
    int _index;
    struct _frame_node* next;
    struct _frame_node* prev;
    //struct list_head list;
}_frame_node;
//_frame_node* frame_freelist[MAX_CONTIBLOCK_SIZE+1];
enum _frame_status{
    _R=-1,
    _F=-2,
    _X=-3
};
#define FRAME_ARRAY_SIZE (ALLOCATOR_END-ALLOCATOR_START)/BLOCK_SIZE
//int frame_array[FRAME_ARRAY_SIZE];
void init_frame_freelist();
void* get_freeframe_addr(unsigned int);
_frame_node* create_frame_node(int idx,int exp,_frame_node* next,_frame_node* prev);
void frame_list_addtail(int i,_frame_node *element);
void free(void* frame_addr);
int find_buddy(int,int);
void merge_buddy();
void _F_frame_array(int idx, int exp);
void _X_frame_array(int idx, int exp);


typedef struct _frame_chunk_node
{
    void* addr;
    struct _frame_chunk_node* next;

}_frame_chunk_node;

typedef struct _frame_array_node
{
    int size;
    struct _frame_node* node;
}_frame_array_node;

void* my_malloc(unsigned int);
void* get_freechunk_addr(int size);
void chunkNode_append(void* addr);
int getBestChunkSize(int require_size);
unsigned int frameAddrToIdx(void* frame_addr);
void list_freeChunkNode();
void list_freeFrameNode();
void memory_reserve(unsigned long long start, unsigned long long end);
void init_memory();
void pop_frame(int idx);

#endif
