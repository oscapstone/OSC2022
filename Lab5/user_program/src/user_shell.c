#include "start.h"
#define BUF_SIZE 100

int main(){
    int pid = getpid();
    
    //test("asdfasdf", 420);
    //uart_write("aa\n", 420);
    //uart_write("bb\n", 420);
    //uart_write("pid: ", pid);
    //uart_read(buff, 100);
    //uart_write("read result: \n", 420);
    //uart_write(buff);
    //uart_write()
    uart_write("pid: \n", 7);
    print_i(getpid());
    //uart_write("fork test\n");
    //fork_test();
    
    while(1){
        char buff[100];
        int read_len = uart_read(buff, 100);
        uart_write("read result: \n", 16);
        uart_write(buff, read_len);
        uart_write(", read len: ", 12);
        print_i(read_len);
        uart_write("\n", 1);
    }
    return 0;
}