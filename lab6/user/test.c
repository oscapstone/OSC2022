#include "user_lib.h"
#include "types.h"
uint64_t cnt = 1;
void main(void){
    int ret = 0;
    uint64_t parent_pid = getpid();
    cnt = 0;
    printf("Start fork\r\n");
    if ((ret = fork()) == 0) { 
        cnt = 0;
        for(int32_t i = 0 ; i < 20 ; i++){
            printf("%d: child pid: %d, cnt: %l\r\n",i, getpid(), cnt);
            cnt++;
        }
    }else {
        cnt = 0;
        for(int32_t i = 0 ; i < 20 ; i++){
            printf("%d: Parent pid: %d, cnt: %l\r\n",i, getpid(), cnt);
            cnt++;
        }
    }
   // trigger seg fault
   for(uint64_t i = 0 ; i < 100000000 ; i++);
   *((uint64_t*)1000000) = 1;
}


