#include "uart.h"
#include "string.h"
#include "reboot.h"
#include "mailbox.h"
#include "mmio.h"
#include "printf.h"
#include "cpio.h"

void shell(){
  char input[20] = "";
  while(1){
    char read = 0;
    read = uart_getc();
    if(read != '\n' && read != 0x7f){
      append_str(input, read);
      printf("%c", read);
      read = 0;
    }else if(read == 0x7f){
      if(strlen(input) > 0){
        pop_str(input);
        printf("\b \b");
      }
    }else{
      if(strlen(input) != 0){
        printf("\n\r");
        if(!strcmp(input, "help")){
          printf("help      : print this help menu\n\r");
          printf("hello     : print Hello World!\n\r");
          printf("reboot    : reboot the device\n\r");
          printf("sysinfo   : print the system information\n\r");
          printf("ls        : list the file\n\r");
          printf("cat <file>: print the content of the file\n\r");
        }else if(!strcmp(input, "hello")){
          printf("Hello World!\n\r");
        }else if(!strcmp(input, "reboot")){
          printf("Bye bye~\n\r");
          reset(300);
        }else if(!strcmp(input, "sysinfo")){
          if(mailbox_property(MAILBOX_TAG_BOARD_VISION, 4, MBOX_CH_PROP)){
            printf("Board version: 0x%08x\n\r", mbox[5]);
          }else{
            printf("Board version: querry error.\n\r");
          }
          if(mailbox_property(MAILBOX_TAG_MEMORY, 8, MBOX_CH_PROP)){
            printf("ARM memory base address: 0x%08x\n\r", mbox[5]);
            printf("ARM memory size: 0x%08x\n\r", mbox[6]);
          }else{
            printf("ARM memory base address: querry error.\n\r");
            printf("ARM memory size: querry error.\n\r");
          }
        }else if(!strcmp(input, "ls")){
          cpio_ls();
        }else if((input[0] == 'c') && (input[1] == 'a') && (input[2] == 't') && (input[3] == 0x20)){
          cpio_cat(input+4);
        }else{
          printf("Please use \"help\" to get information.\n\r");
        }
        printf("raspberryPi: ");
        input[0] = 0;
      }else{
        printf("\n\rraspberryPi: ");
      }  
    }
  }
}