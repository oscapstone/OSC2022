#include "mini_uart.h"
#include "shell.h"
#include "stdlib.h"
extern unsigned long _head_start_brk;
int main(){
    init_uart();
    writes_uart("\r\n");
    writes_uart("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\r\n");
    writes_uart("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\r\n");
    writes_uart("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗  \r\n");
    writes_uart("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝  \r\n");
    writes_uart("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\r\n");
    writes_uart("╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\r\n");
    writehex_uart(&_head_start_brk);
    char* string = simple_malloc(8);
    string[0]=1;
    string[1]=2;
    string[2]=3;
    string[3]=4;
    string[4]=5;                                            
    while(1){
        //writec_uart(read_uart());
        
        read_command();
    }
    return 0;
}