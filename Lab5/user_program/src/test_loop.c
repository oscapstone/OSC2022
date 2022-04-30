#include "start.h"

int main(){
    print_s("User Process: test loop\r\n");
    char buff[1000];
    while(1){
        print_s("looping: \r\n");
        //delay(1000000000);
        getpid();
        int len = uart_read(buff, 100);
        print_s("read result: \r\n");
        uart_write(buff, len);
    }
    return 0;
}