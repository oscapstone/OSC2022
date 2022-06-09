#include "user_lib.h"

size_t uart_read(char *buf, size_t size){
    asm volatile("mov x0, %[buf]\n\t"
                 "mov x1, %[size]\n\t"
                 "mov x8, 1\n\t"
                 "svc 0\n\t"
                 :
                 :[buf] "r" (buf), [size] "r" (size)
    );
}

size_t uart_write(char *buf, size_t size){
    asm volatile("mov x0, %[buf]\n\t"
                 "mov x1, %[size]\n\t"
                 "mov x8, 2\n\t"
                 "svc 0\n\t"
                 :
                 :[buf] "r" (buf), [size] "r" (size)
    );
}



