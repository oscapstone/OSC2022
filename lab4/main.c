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
  

  page_allocate(2*0x1000);
  int all = page_allocate(1*0x1000);


  print_frame_state();
  print_free_frame_list();
  printf("index: %d\n\r", all);

  page_free(all);
  print_frame_state();
  print_free_frame_list();


  printf("\n\r\n\rWelcome!!!\n\r");
  printf("raspberryPi: ");

  shell();
}
