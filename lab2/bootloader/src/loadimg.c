#include "mini_uart.h"
#include "string.h"

#define byte unsigned char
#define uint32 unsigned int
#define block_size 128


void loadimg(){
    uint32 addr=0x80000; // load to 0x80000
    uart_printf("Start receiving.\n");
    byte SIZE[5];
    byte par=0;
    for(int i=0;i<5;i++){  //receive file size
        byte temp=uart_read_raw();
        SIZE[i]=temp;
        if(i!=4) par ^= temp;
    }
    if(par != SIZE[4]){
        uart_printf("Receiving error\n");
        return;
    }
    uint32 size=0;
    for(int i=0;i<4;i++){ 
        size <<= 8;
        size += SIZE[i];
    }
    byte *kernel = (byte*) addr;
    byte check_byte=0;
    int j=0;
    for(int i=0;i<size;i++){ //receive file
        byte c=uart_read_raw();
        *(kernel+i)=c;
        check_byte ^= c;
        if(i%8 == 7){ //check whether there is any error
            byte temp=uart_read_raw();
            if(check_byte != temp){
                if(j++>=5){
                    uart_printf("Error! Please try again later.\n");
                    for(int j=0;j<5000;j++) asm volatile("nop");
                    uart_write(2);
                    return;
                }
                uart_printf("Something went wrong. Trying to fix it. %d\n",i/8);
                i-=8;
                for(int j=0;j<5000;j++) asm volatile("nop");
                uart_write(1);
            }else{
                j=0;
                check_byte &= 0;
                for(int j=0;j<5000;j++) asm volatile("nop");
                uart_write(0);
            }
        }
    }
    void (*start_os)(void) = (void *)kernel; //set start_os's addr to the loaded image's addr
    uart_printf("Loading Complete.\n");
    uart_printf("Redirecting to 0x%x\n",addr);
    start_os();
}