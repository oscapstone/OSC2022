#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "malloc.h"
#include "dtb.h"
#include "timer.h"

extern unsigned long _kernel_start;
extern unsigned long _kernel_end;

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
  print_free_frame_list();
  memory_reserve(0x0000, 0x1000); //spin table
  memory_reserve((unsigned long)&_kernel_start, (unsigned long)&_kernel_end);
  memory_reserve(0x20000000, 0x20000000+69120);  // cpio size

  print_free_frame_list();
  printf("\n\r\n\rWelcome!!!\n\r");
  printf("raspberryPi: ");

  shell();
}
