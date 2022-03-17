#include "uart.h"
#include "printf.h"

extern char* _code_relocate_place;
extern unsigned long long  __code_size;
extern unsigned long long _start;

void code_relocate(char * addr, char * dtb);

int relocate=1;

void main(char* arg){
  // register char* x0 asm("x0");
  
  char *_dtb = arg;
  uart_init();
  char* reloc_place = (char*)&_code_relocate_place;

  if(relocate){
    relocate = 0;
    code_relocate(reloc_place, _dtb);
  }

  unsigned long long kernel_size = 0;
  char* kernel_start = (char*) 0x80000;
  char c;
  printf("Welcome UART bootloader!!\n\rPlease input the kernel size:\n\r");
  for(int i = 0; i < 4; i++){
    c = uart_getc_pure();
    kernel_size += c<<(i*8);
  }
  for(int i = 0; i < kernel_size; i++){
    c = uart_getc_pure();
    kernel_start[i] = c;
  }
  void (*kernel) (char *) = (void (*) (char *))kernel_start;
  kernel(arg);
}

void code_relocate(char * addr, char * dtb){
  unsigned long long size = (unsigned long long)&__code_size;
  char* start = (char *)&_start;
  for(unsigned long long i=0; i<size; i++){
    addr[i] = start[i];
  }
  void (*relocation)(char *)  = (void (*) (char *))addr; 
  relocation(dtb);
}
