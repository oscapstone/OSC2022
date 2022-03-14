#include <uart.h>
#include <shell.h>
#include <cpio.h>

int main(){
    uart_init();
    PrintWelcome();
    ShellLoop();
    
    return 0;
}