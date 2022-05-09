#include "types.h"
#include "utils.h"
char buf[0x2000];
void simple_shell(){
    unsigned int i;
    char ch;
    while(1){
        i = 0;
        printf("# ");
        // read command
        while(1){
            ch = (char)getchar();
            if(ch != '\r'){
                putchar(ch);
                buf[i] = ch;        
                i++;
            }else{
                printf("\r\n");
                buf[i] = '\0';
                break;
            }
        }
        // match comman
        if(strcmp(buf, "help") == 0){
            printf("help    : print this help menu\r\n");
            printf("hello   : print Hello World!\r\n");
            printf("mailbox : print hardware infomation\r\n");
            printf("reboot  : reboot the device\r\n");
        }else if(strcmp(buf, "hello") == 0){
            printf("Hello World!\r\n");
        }else if(strcmp(buf, "mailbox") == 0){

        }else if(strcmp(buf, "reboot") == 0){
            
        }
    }
} 
