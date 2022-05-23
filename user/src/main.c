#include "mini_uart.h"
extern int get_pid();
extern unsigned int uart_read(char buf[],unsigned int size);
extern unsigned int uart_write(const char* name,unsigned int size);
extern int exec(const char* name, char *const argv[]);
extern int fork();
extern void exit();
extern int mbox_call(unsigned char ch, unsigned int *mbox);
extern void kill(int pid);
void delay(int time)
{
    unsigned long long delay_time = time*0x1000;
    while(delay_time>1){
        delay_time--;
    }
}
void fork_test(){
    // printf("\nFork Test, pid %d\n", get_pid());
    busy_wait_writes("\nFork Test, pid ",FALSE);
    busy_wait_writeint(get_pid(),TRUE);
    int cnt = 1;
    int ret = 0;
    if ((ret = fork())== 0) { // child
        long long cur_sp;
        
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        // printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
        busy_wait_writes("first child pid: ",FALSE);
        busy_wait_writeint(get_pid(),FALSE);
        busy_wait_writes(", cnt: ",FALSE);
        busy_wait_writeint(cnt,FALSE);
        busy_wait_writes(", ptr: ",FALSE);
        busy_wait_writehex((unsigned long long )(&cnt),FALSE);
        busy_wait_writes(", sp: ",FALSE);
        busy_wait_writehex((unsigned long long)cur_sp,TRUE);
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            // printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
            busy_wait_writes("first child pid: ",FALSE);
            busy_wait_writeint(get_pid(),FALSE);
            busy_wait_writes(", cnt: ",FALSE);
            busy_wait_writeint(cnt,FALSE);
            busy_wait_writes(", ptr: ",FALSE);
            busy_wait_writehex((unsigned long long )(&cnt),FALSE);
            busy_wait_writes(", sp: ",FALSE);
            busy_wait_writehex((unsigned long long)cur_sp,TRUE);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                // printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
                busy_wait_writes("second child pid: ",FALSE);
                busy_wait_writeint(get_pid(),FALSE);
                busy_wait_writes(", cnt: ",FALSE);
                busy_wait_writeint(cnt,FALSE);
                busy_wait_writes(", ptr: ",FALSE);
                busy_wait_writehex((unsigned long long )(&cnt),FALSE);
                busy_wait_writes(", sp: ",FALSE);
                busy_wait_writehex((unsigned long long)cur_sp,TRUE);
                delay(1000000);
                ++cnt;
            }
        }
        exit();
    }
    else {
        // printf("parent here, pid %d, child %d\n", get_pid(), ret);
        busy_wait_writes("parent here, pid ",FALSE);
        busy_wait_writeint(get_pid(),FALSE);
        busy_wait_writes(", child ",FALSE);
        busy_wait_writeint(ret,TRUE);
        
    }
    exit();
    // while(1){asm volatile("nop");}
}
int main(){
    // while(1)
    // {
    //     busy_wait_writes("HI, here is user ",FALSE);
    //     busy_wait_writeint(get_pid(),TRUE);
    // }
    fork_test();
    exit();
    return 0;
}