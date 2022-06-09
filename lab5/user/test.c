#include "user_lib.h"
#include "types.h"


int __start(void){
    char buf[15];
    //size_t s;
    uart_write("test fork 1\r\n", sizeof("test fork 1\r\n"));
    uart_write("> \r\n", sizeof(">\r\n"));
    uart_read(buf, 1);
    uart_write("start fork\r\n", sizeof("start fork\r\n"));
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


