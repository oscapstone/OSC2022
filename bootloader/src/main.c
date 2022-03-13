#include "mini_uart.h"
#include "shell.h"
#include "bootload.h"
extern unsigned int _relocate_addr;
extern unsigned int _start;
extern unsigned int _end;
int relocated=1;
int main(){


    // unsigned int reloc_place = (char*)&_rel_addr;
    // char* start_place = (char*)&_start;
    // char* end_place = (char*)&_end;

    if(relocated==1){
        relocated=0;
        relocate(&_relocate_addr);
        }
    init_uart();
    writes_uart("\r\n");
    writes_uart("Here is bootloader\r\n");
    // writes_uart("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\r\n");
    // writes_uart("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\r\n");
    // writes_uart("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗  \r\n");
    // writes_uart("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝  \r\n");
    // writes_uart("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\r\n");
    // writes_uart("╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\r\n");
    while(1){
        //writec_uart(read_uart());
        read_command();
        
        //writehex_uart((unsigned int)&_start);
        
        // if(relocated==0){
        //     relocated=1;
            
        // }else{
        //     writes_uart("Already relocated\r\n");
        // }
        
        // if(_start != relocate_addr){
        //     writes_uart("Relocating bootloader to 0x60000\r\n");
        //     relocate(&_relocate_addr);
        // }else{
        //     writes_uart("Sucessful relocate\r\n");
        // }

        // writehex_uart((unsigned int)&_rel_addr);
        // writes_uart("\r\n");
        // writehex_uart((unsigned int)start_place);
        // writes_uart("\r\n");
        // writehex_uart((unsigned int)end_place);
        // writes_uart("\r\n");
        // writehex_uart((unsigned int)(_end - _start));
        // writes_uart("\r\n");
    }
    return 0;
}