#include "user_lib.h"
#include "types.h"


int __start(void){
    printf("\r\n%s, pid %d\r\n","Fork test", getpid());
    int cnt = 1;
    int ret = 0;
    //debug_info();
    if ((ret = fork()) == 0) { // child
        //debug_info();
        long long cur_sp;
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
        printf("parent here, pid %d, child %d\r\n", getpid(), ret);
    } 
    while(1);
}


