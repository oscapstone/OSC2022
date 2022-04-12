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
    "msr DAIFClr, 0xf" // enable interrupt el1 -> el1
  ); 
  core_timer_enable();
  fdt_traverse(initramfs_callback);
  page_init();


  printf("\n\r\n\rWelcome!!!\n\r");
  printf("raspberryPi: ");

  shell();
}
