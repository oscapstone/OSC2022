#include "mini_uart.h"
#include "shell.h"
#include "mini_uart.h"
#include "string.h"
#include "bootload.h"
extern unsigned int _relocate_addr;
extern unsigned int _start;
extern unsigned int _end;
int relocated=1;
unsigned long dtb_addr;// asm ("x15");

int main(){


    // unsigned int reloc_place = (char*)&_rel_addr;
    // char* start_place = (char*)&_start;
    // char* end_place = (char*)&_end;
    
    if(relocated==1){
        register unsigned long dtb_reg asm ("x15"); // the x0 has been stored into x15 at boot.S _start.
        dtb_addr = dtb_reg;
        relocated=0;
        relocate(&_relocate_addr);
    }
    
    init_uart();
    writes_uart("\r\n");
    writes_uart("Here is bootloader\r\n");
    
    char buffer[25600];
    int count=0;
    writes_uart("# ");
    while(1){
        char c = read_uart();
        if(c!='\n' && count<256){
            if(c>=0 && c<=127){ // only accept the value is inside 0~127(ASCII).
               writec_uart(c);
                buffer[count++] = c; 
            }
        }else{
            writec_uart('\r');
            writec_uart('\n');
            buffer[count]='\0';
            if(buffer[0] != '\0'){
                if(strcmp(buffer,"bootload")==0){
                    writes_uart("Start loading the image file.\r\n");
                    bootload_image(dtb_addr);
                }
            }
            break;
        }
        
    }
    
    return 0;
}