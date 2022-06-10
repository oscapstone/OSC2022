#include "user_lib.h"
#include "types.h"


int __start(void){
    int cnt = 1;
    int ret = 0;
    long long cur_sp;
                asm volatile("mov %0, sp" : "=r"(cur_sp));
    printf("Fork test pid: %d, cnt: %d, ptr: %x, sp : %x\r\n", getpid(), cnt, &cnt, cur_sp);
    //debug_info();
    if ((ret = fork()) == 0) { // child
        //debug_info();
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\r\n", getpid(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = fork()) != 0){
            //debug_info();
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\r\n", getpid(), cnt, &cnt, cur_sp);
        }
        else{
            //debug_info();
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\r\n", getpid(), cnt, &cnt, cur_sp);
                delay(1000000);
                ++cnt;
            }
        }
    }
    else {
            //debug_info();
                asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("parent here, pid: %d, cnt: %d, ptr: %x, sp : %x\r\n", getpid(), cnt, &cnt, cur_sp);
    } 
    while(1);
}


