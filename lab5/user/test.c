#include "user_lib.h"
#include "types.h"


int __start(void){
    char buf[15];
    //size_t s;
    uart_write("hello world\r\n", sizeof("hello world\r\n"));
    while(1){
        if(fork() == 0){
            while(1){
                uart_write("hello child\r\n", sizeof("hello child\r\n"));
                delay(500000);
            }
        }else{
            while(1){
                uart_write("hello parent\r\n", sizeof("hello parent\r\n"));
                delay(500000);
            }
        }

    }
}


