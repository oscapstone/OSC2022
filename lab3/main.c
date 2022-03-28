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
  core_timer_enable();
  fdt_traverse(initramfs_callback);

  printf("\n\r\n\rWelcome!!!\n\r");
  printf("raspberryPi: ");
  shell();
}
