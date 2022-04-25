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
    
    while(1){
        char buff[100];
        uart_read(buff, 100);
        uart_write("read result: \n", 420);
        uart_write(buff, 420);
        uart_write("\n", 420);
    }
    return 0;
}