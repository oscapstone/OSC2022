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


  memory_reserve(0xaad50, 0xafd50);
  // memory_reserve(15, 52);
  // print_frame_state();
  // print_free_frame_list();

  printf("\n\r\n\rWelcome!!!\n\r");
  printf("raspberryPi: ");

  shell();
}
