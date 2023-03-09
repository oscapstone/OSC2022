#include "user_lib.h"
#include "types.h"
void sig_handler_9(){
    printf("hello world signal 9\r\n");
}

void sig_handler_31(){
    printf("hello world signal 31!\r\n");
}

void test1(){
    char a[0x3000];
    a[1000] = '0';
    a[7000] = '0';
}
void test2(){
    *((uint32_t*)0x0) = 0x1e;
}
void main(void){
    long cnt = 1;
    int ret = 0;
    uint64_t parent_pid = getpid();
    signal(9, sig_handler_9);
    signal(31, sig_handler_31);
    if ((ret = fork()) == 0) { 
        test2();
        printf("child pid: %d\r\n", getpid());
        for(int32_t i = 0 ; i < 60 ; i++){
            if(i != 7 && i != 11)
                kill(parent_pid, i);
        }
        *((uint64_t*)0x1000000) = 0;
    }else {
        test2();
        printf("Parent pid: %d\r\n", getpid());
        for(int32_t i = 0 ; i < 60 ; i++){
            if(i != 7 && i != 11)
                kill(ret, i);
        }
        test1();
        for(cnt = 0 ; cnt < 1000000000 ; cnt++);
        *((uint64_t*)0x1000000) = 0;
    }
    while(1);
}


