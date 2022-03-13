#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "malloc.h"
#include "dtb.h"

extern char* dtb_place;

void main(char * arg){

  dtb_place = arg;
  uart_init();

  fdt_traverse(initramfs_callback);
  

  // Test the simple_malloc
  char* test1 = simple_malloc(0x18);
  test1 = "Test the malloc 1.";
  printf("%s\n\r", test1);
  char* test2 = simple_malloc(0x32);
  test2 = "Test the malloc 2.";
  printf("%s\n\r", test2);
  char* test3 = simple_malloc(0x36);
  test3 = "Test the malloc 3.";
  printf("%s\n\r", test3);

  printf("\n\r\n\rWelcome!!!\n\r");
  printf("raspberryPi: ");
  
  shell();
}
