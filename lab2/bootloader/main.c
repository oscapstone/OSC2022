#include "uart.h"
#include "printf.h"

void main()
{
  uart_init();
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
  void (*kernel) (void) = (void (*) (void))kernel_start;
  kernel();
}
