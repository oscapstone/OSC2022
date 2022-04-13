// #include "list.h"
#ifndef __ALLCATOR_H
#define __ALLCATOR_H

#define ALLOCATOR_BASE 0x10000000
#define ALLOCATOR_START (volatile unsigned int)(ALLOCATOR_BASE)
#define ALLOCATOR_END (volatile unsigned int)(ALLOCATOR_BASE+ 0x10000000)
#define BLOCK_SIZE 0x1000
#define MAX_CONTIBLOCK_SIZE 16

typedef struct _frame_node
{
    int _block_exponent;
    int _index;
    struct _frame_node* next;
    //struct list_head list;
}_frame_node;
_frame_node* frame_freelist[MAX_CONTIBLOCK_SIZE+1];
enum _frame_status{
    _X=-1,
    _F=-2
};
#define FRAME_ARRAY_SIZE (ALLOCATOR_END-ALLOCATOR_START)/BLOCK_SIZE
int frame_array[FRAME_ARRAY_SIZE];
void init_frame_freelist();
void* get_freeframe_addr(unsigned int);
_frame_node* create_frame_node(int idx,int exp,_frame_node* next);
void frame_list_addtail(int i,_frame_node *element);
void free_frame(unsigned long long frame_addr);
int find_buddy(int,int);
void merge_buddy();
void _F_frame_array(int idx, int exp);
void _X_frame_array(int idx, int exp);

typedef struct _frame_chunk_node
{
    unsigned int _chunk_addr;
    struct _frame_chunk_node* next;

}_frame_chunk_node;
void* my_malloc(unsigned int);

#endif
