#include "allocator.h"
#include "stdlib.h"
#include "mini_uart.h"
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
    return (ALLOCATOR_START + node->_index*BLOCK_SIZE);
    
}
int find_buddy(int frame_idx, int frame_exp){
    if(frame_exp==16)
        return frame_idx;
    int buddy_frame_idx = frame_idx^(power(2,frame_exp));
    return buddy_frame_idx;
}
void merge_buddy(int frame_idx, int buddy_idx){
    if(frame_idx == buddy_idx || frame_array[frame_idx]<BLOCK_SIZE
                              || frame_array[buddy_idx]<BLOCK_SIZE)
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
void free_frame(unsigned long long frame_addr){
    //writehex_uart(frame_addr-ALLOCATOR_START,TRUE);
    int frame_idx = (frame_addr-ALLOCATOR_START)/BLOCK_SIZE;
    if(frame_array[frame_idx]>=_X)
        return;
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
}

// int frameAddrToIdx(unsigned int frame_addr){
//     return (frame_addr-ALLOCATOR_START)/BLOCK_SIZE;
// }
// int chunk_size_arr[] = {0x1,
//                         0x2,
//                         0x4,
//                         0x8,
//                         0x10,
//                         0x20,
//                         0x40,
//                         0x80,
//                         0x100,
//                         0x200,
//                         0x400,
//                         0x800};
// int chunk_arr_len = 12;
// int getBestChunkSize(int require_size){
//     for (int i = chunk_arr_len-1; i >=0; i--)
//     {
//         if(require_size / chunk_size_arr[i]>=1){
//             return chunk_size_arr[i];
//         }
//     }
    
// }
// unsigned int* get_freechunk_addr(int size){

// }
// void* my_malloc(unsigned int size){
//     if(size<BLOCK_SIZE){
//         unsigned int* chunk_start_addr = get_freechunk_addr(size);
//         if(chunk_start_addr == nullptr)
//             chunk_start_addr = get_freeframe_addr(BLOCK_SIZE);
//         int chunk_frame_idx = frameAddrToIdx(chunk_start_addr);
//         int chunk_size = getBestChunkSize(size);
//         frame_array[chunk_frame_idx] = chunk_size;
//     }
//     else{

//     }
// }