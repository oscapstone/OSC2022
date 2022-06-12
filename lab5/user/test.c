#include "user_lib.h"
#include "types.h"
void sig_handler_9(){
    printf("hello world signal 9\r\n");
}

void sig_handler_31(){
    printf("hello world signal 31!\r\n");
}



void main(void){
    int cnt = 1;
    int ret = 0;
    uint64_t parent_pid = getpid();
    signal(9, sig_handler_9);
    signal(31, sig_handler_31);
    if ((ret = fork()) == 0) { 
        printf("child pid: %d\r\n", getpid());
        for(int32_t i = 0 ; i < 60 ; i++){
            kill(parent_pid, i);
        }
    }else {
        printf("Parent pid: %d\r\n", getpid());
        for(int32_t i = 0 ; i < 60 ; i++){
            kill(ret, i);
        }
    }
    while(1);
}


