#include "uart.h"
#include "string.h"
#include "reboot.h"
#include "mailbox.h"
#include "mmio.h"

void shell(char *input){
  char read = 0;
  read = uart_getc();
  if(read != '\n' && read != 0x7f){
    append_str(input, read);
    uart_send(read);
    read = 0;
  }else if(read == 0x7f){
    pop_str(input);
    uart_puts("\b \b");
  }else{
    uart_puts("\n");
    if(read == '\n'){ 
      if(!strcmp(input, "help")){
        uart_puts("help    : print this help menu\n");
        uart_puts("hello   : print Hello World!\n");
        uart_puts("reboot  : reboot the device\n");
        uart_puts("sysinfo : print the system information\n");
      }else if(!strcmp(input, "hello")){
        uart_puts("Hello World!\n");
      }else if(!strcmp(input, "reboot")){
        uart_puts("Bye bye~\n");
        reset(100);
      }else if(!strcmp(input, "sysinfo")){
        if(mailbox_property(MAILBOX_TAG_BOARD_VISION, 4, MBOX_CH_PROP)){
          uart_puts("Board version: ");
          uart_hex(mbox[5]);
          uart_puts("\n");
        }else{
          uart_puts("Board version: querry error.\n");
        }
        if(mailbox_property(MAILBOX_TAG_MEMORY, 8, MBOX_CH_PROP)){
          uart_puts("ARM memory base address: ");
          uart_hex(mbox[5]);
          uart_puts("\n");
          uart_puts("ARM memory size: ");
          uart_hex(mbox[6]);
          uart_puts("\n");
        }else{
          uart_puts("ARM memory base address: querry error.\n");
          uart_puts("ARM memory size: querry error.\n");
        }
      }else{
        uart_puts("Please use \"help\" to get information.\n");
      }
      input[0] = 0;
    }
  }
}