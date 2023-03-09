#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <time.h>
#include "lib/simple_malloc.h"

#define RANGE 1000
int main(void){
    void* cur,* next;
    size_t req_size, real_size;
    void* p = mmap( NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );
    init_malloc_state(p);
    srand((unsigned)time(0));
    
    for(int i = 0 ; i < 30 ; i++){
        req_size = (uint64_t)rand() % RANGE;
        cur = simple_malloc(req_size);
        next = get_remainder();
        real_size = (uint64_t)next - (uint64_t)cur; 
        printf("cur: %p, next: %p, request size: %lu, real size: %lu, legal: %d\n", cur, next , req_size, real_size, (req_size <= real_size) && !(real_size % 16));
    }

    return 0;
}
