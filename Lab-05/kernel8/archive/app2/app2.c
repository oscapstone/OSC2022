#include "inc/syscall.h"

int main(void) {
	uart_printf("app2\n");
	char buffer[500];
	uart_printf("read test: ");
	int len=uart_read(buffer,500);
    uart_write("write test:", 500);
    uart_write(buffer, len);
    uart_printf("Fork Test, pid %d\n", getpid());
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        uart_printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            uart_printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                uart_printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                delay(1000000);
                ++cnt;
            }
        }
    }else {
        uart_printf("parent here, pid %d, child %d\n", getpid(), ret);
    }
    exit();
}