#include "ring_buffer.h"
#include <stdio.h>
#include <stdlib.h>
char* s = "abcdefghijklmn";
int main(void){
    char b[10];
    ring_buffer* rbuf = create_ring_buf(10);
    if(ring_buf_is_empty(rbuf)){
        printf("Ring buffer is empty now!\n");
    }

    ring_buf_write(rbuf, s, 8);
    
    if(!ring_buf_is_empty(rbuf)){
        printf("Ring buffer is has some contant now!\n");
        int i = ring_buf_read(rbuf, b, 100);
        b[i] = '\0';

        printf("%d: %s\n", i, b);
        if(ring_buf_is_empty(rbuf)){
            printf("Ring buffer is empty now!\n");
        }
    }
    printf("%lu\n", ring_buf_write(rbuf, s, 16));
    if(ring_buf_is_full(rbuf)){
        printf("Ring buffer is full now!\n");
    }

    int i = ring_buf_read(rbuf, b, 4);
    b[i] = '\0';
    printf("%d: %s\n", i, b);

    i = ring_buf_write(rbuf, b, 10);
    printf("%d\n", i);
    
    i = ring_buf_read(rbuf, b, 7);
    b[i] = '\0';
    printf("%d: %s\n", i, b);
    
    i = ring_buf_read(rbuf, b, 10);
    b[i] = '\0';
    printf("%d: %s\n", i, b);

    if(ring_buf_is_full(rbuf)){
        printf("Ring buffer is full now!\n");
    }
    if(ring_buf_is_empty(rbuf)){
            printf("Ring buffer is empty now!\n");
    }
}
