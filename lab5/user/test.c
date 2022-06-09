#include "user_lib.h"
#include "types.h"
int main(void){
    char buf[15];
    size_t s;
    uart_write("hello world\r\n", sizeof("hello world\r\n"));
    while(1){
        s = uart_read(buf, 10);
        uart_write(buf, s);
        uart_write("\r\n", 2);
    }
}
