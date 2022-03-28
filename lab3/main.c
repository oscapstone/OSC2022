#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "malloc.h"
#include "dtb.h"
#include "timer.h"

extern char* dtb_place;

void main(char * arg){
  dtb_place = arg;
  uart_init();
  __asm__ __volatile__(
    "msr daifclr, 0xf" // enable interrupt el1 -> el1
  ); 
  fdt_traverse(initramfs_callback);

  printf("\n\r\n\rWelcome!!!\n\r");
  printf("raspberryPi: ");
  shell();
}
