#include "uart.h"
#include "shell.h"
#include "printf.h"

void main()
{
  uart_init();

  printf("\n\r\n\rWelcome!!!\n\r");
  printf("raspberryPi: ");

  shell();
}
