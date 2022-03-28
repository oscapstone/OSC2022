#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "malloc.h"
#include "dtb.h"

extern char* dtb_place;

void main(char * arg){
  dtb_place = arg;
  uart_init();

  // fdt_traverse(initramfs_callback);

  uart_puts("\n\r\n\rWelcome!!!\n\r");
  printf("raspberryPi: ");
  char input[20] = "";
  shell(input);
}
