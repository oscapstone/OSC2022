#include "shell.h"
#include "string.h"
#include "mailbox.h"
#include "reboot.h"
#include "cpio.h"
#include "stdlib.h"
#include "timer.h"
int match_command(char *buffer){
    
    if(strcmp(buffer,"help")==0){
        return help;
    }
    else if(strcmp(buffer,"hello")==0){
        return hello;
    }else if(strcmp(buffer,"revision")==0){
        return revision;
    }else if(strcmp(buffer,"memory")==0){
        return memory;
    }
    else if(strcmp(buffer,"reboot")==0){
        return reboot;
    }
    else if(strcmp(buffer,"ccreboot")==0){
        return ccreboot;
    }
    else if(strcmp(buffer,"version")==0){
        return version;
    }
    else if(strcmp(buffer,"ls")==0){
        return ls;
    }
    else if(strcmp(buffer,"cat")==0){
        return cat;
    }
    else if(strcmp(buffer,"prog")==0){
        return prog;
    }
    else if(strncmp(buffer,"timeout",strlen("timeout"))==0){
        return timeout;
    }
    else{
        return unknown;
    }
}


void read_command(){
    char buffer[512];
    int count=0;
    int action = 0;
    writes_uart("# ");
    while(1){
        do{asm volatile("nop");}while(is_empty_read());
        char c = uart_buf_read_pop();
        // writehex_uart(uart_write_i_l,0);
        // writehex_uart(uart_write_i_r,1);
        if(c!='\n' && count<256){
            if(c=='\x7f'){
                if(count >0 ){
                    uart_buf_write_push('\b');
                    uart_buf_write_push(' ');
                    uart_buf_write_push('\b');
                    buffer[--count]='\0';
                }
                
            }
            else if(c>=0 && c<=127){ // only accept the value is inside 0~127(ASCII).
                uart_buf_write_push(c);
                buffer[count++] = c;
                buffer[count] = '\0';
            }
            
        }else{
            uart_buf_write_push('\r');
            uart_buf_write_push('\n');
            buffer[count]='\0';
            if(buffer[0] != '\0'){
                action = match_command(buffer);
                handle_command(action, buffer);
            }
            break;
        }
        
    }
    return;
}

// void read_command(){
//     char buffer[256];
//     int count=0;
//     int action = 0;
//     writes_uart("# ");
//     while(1){
//         char c = read_uart();
//         if(c!='\n' && count<256){
//             if(c=='\x7f'){
//                 if(count >0 ){
//                     writec_uart('\b');
//                     writec_uart(' ');
//                     writec_uart('\b');
//                     buffer[--count]='\0';
//                 }
                
//             }
//             else if(c>=0 && c<=127){ // only accept the value is inside 0~127(ASCII).
//                 writec_uart(c);
//                 buffer[count++] = c; 
//             }
            
//         }else{
//             writec_uart('\r');
//             writec_uart('\n');
            
//             buffer[count]='\0';
//             if(buffer[0] != '\0'){
//                 action = match_command(buffer);
//                 handle_command(action, buffer);
//             }
//             break;
//         }
        
//     }
//     return;
// }
void read_string(char **s){
    char *buffer= simple_malloc(256);
    int count=0;
    while(1){
        do{asm volatile("nop");}while(is_empty_read());
        char c = uart_buf_read_pop();
        if(c!='\n' && count<256){
            if(c=='\x7f'){
                if(count >0 ){
                    writec_uart('\b');
                    writec_uart(' ');
                    writec_uart('\b');
                    buffer[--count]='\0';
                }
                
            }
            else if(c>=0 && c<=127){ // only accept the value is inside 0~127(ASCII).
                writec_uart(c);
                buffer[count++] = c; 
            }
            
        }else{
            writec_uart('\r');
            writec_uart('\n');
            
            buffer[count]='\0';
            if(buffer[0] != '\0'){
                *s = buffer;
                return;
            }
            break;
        }
        
    }
    return;
}
int read_int(){
    // int n=0;
    // for(int i=0;i<4;i++){
    //     char c = read_uart();
    //     n = n << 8;
    //     n += (int) c;
    // }
    // return n;
    char* s;
    read_string(&s);
    int n=0;
    for (int i = 0; i < strlen(s); i++)
    {
        char c = s[i];
        n = n *10;
        n += c-'0';
        /* code */
    }
    return n;
    
}

int get_para_num(char *s){
    int n=0;
    for (int i = 0; i < strlen(s); i++)
    {
        if(s[i]==' ') n++;
    }
    return n;
}

char* get_para_by_num(int num,char *buffer){
    int space_num=0;
    int start=0,end=0;
    for (int i = 0; i < strlen(buffer); i++)
    {
        if(buffer[i]==' ')
        {
            space_num++;
            if(space_num==num){
                start = i+1;
            }
            else if(space_num==num+1){
                end = i;
            }
        }
    }
    if(space_num<num)
        return nullptr;
    else
    {
        if(end==0) 
            end = strlen(buffer);
        char *s = simple_malloc(end-start+1);
        s[end-start]='\0';
        int i=0;
        for (i = start; i < end; i++)
        {
            s[i-start] = buffer[i];
        }
        s[i] = '\0';
        return s;
    }
    
}

void handle_command(enum Action action, char *buffer){
    switch (action)
    {
        case help:
            writes_uart("help       : print this help menu\r\n");
            writes_uart("hello      : print Hello World!\r\n");
            writes_uart("revision   : print board revision\r\n");
            writes_uart("memory     : print ARM memory address and size\r\n");
            writes_uart("reboot     : reboot the device after 50000 ticks\r\n");
            writes_uart("ccreboot   : cancel reboot the device\r\n");
            writes_uart("ls         : print the files in rootfs.\r\n");
            writes_uart("cat        : print the file information\r\n");
            writes_uart("usrprog    : load user program and run it\r\n");
            writes_uart("timeout    : set timer timeout events\r\n");
            break;
        case hello:
            writes_uart("Hello World!\r\n");
            break;
        case revision:
            get_board_revision();
            break;
        case memory:
            get_ARM_memory();
            break;
        case reboot:
            writes_uart("reset after 50000 ticks\r\n");
            reset(50000);
            break;
        case ccreboot:
            writes_uart("canceling reset operation...\r\n");
            cancel_reset();
            writes_uart("reset operation has been canceled\r\n");
            break;
        case version:
            writes_uart("Here is your booted kernel ver 0.1\r\n");
            break;
        case ls:
            cpio_ls();
            break;
        case cat:
            cpio_cat();
            break;
        case prog:
        {
            unsigned long long prog_addr = cpio_get_addr();
            unsigned long *e0_stack = (unsigned long*)simple_malloc(2000);
            writehex_uart(prog_addr,1);
            // writehex_uart(e0_stack);
            // writes_uart("\r\n");
            // asm volatile(
            //     "bl core_timer_enable\n\t"
            //     // "bl core_timer_handler\n\t"
            // );
            asm volatile(
                // "mov x0, (1 << 31)\n\t" // EL1 uses aarch64
                // "msr hcr_el21, x0\n\t" // Hypervisor Configuration Register, set RW to 1.
                "mov x0, 0\n\t" // EL1h (SPSel = 1) with interrupt disabled
                "msr spsr_el1, x0\n\t"
                "msr elr_el1, %0\n\t"
                "msr sp_el0,%1\n\t"
                
                "eret\n\t" // return to EL1"
                ::"r" (prog_addr),
                "r" (e0_stack)
                : "x0"
            );
            writes_nl_uart("Run userprogram done");
            break;
        }
        case timeout:
        {
            // writes_uart("Input message:");
            // char *s;
            // read_string(&s);
            // writes_uart("Input time:");
            // unsigned int after = read_int();
            
            // if(get_para_num(buffer)!=2)
            // {
            //     writes_uart("Please input at least 2 parameters\r\n");
            //     return;
            // }
            // else{
            //     //busy_wait_writes(buffer,TRUE);
            //     //busy_wait_writes("add timer shell1",TRUE);
            //     char *message = get_para_by_num(1,buffer);
            //     //busy_wait_writes(message,TRUE);
            //     //busy_wait_writes("add timer shell2",TRUE);
            //     char *after = get_para_by_num(2,buffer);
            //     //busy_wait_writes("add timer shell3",TRUE);
            //     int after_int = str2int(after);
            //     //busy_wait_writes(after,TRUE);
            //     //busy_wait_writes("add timer shell4",TRUE);
            //     add_timer(writes_nl_uart,"3",3);
            // }
            // timeout 3 3


            int space_count=0;
            int message_start=0,message_end=0,after_start=0,after_end=strlen(buffer);
            for (int i = 0; i < strlen(buffer); i++)
            {
                if(buffer[i]==' '){
                    space_count++;
                    if(message_start == 0){
                        message_start = i+1;
                    }
                    else{
                        message_end = i;
                        after_start = i+1;
                    }
                }
            }
            if(space_count!=2){
                writes_uart("Error, please input 'timeout message after'\r\n");
                break;
            }
            //char* message_buffer = simple_malloc(message_end - message_start+1);
            //char* after_buffer = simple_malloc(after_start - after_end+1);
            char* message_buffer = simple_malloc(128);
            char after_buffer[128];
            int i,j;
            for (i = message_start,j=0; i < message_end; i++,j++)
            {
                message_buffer[j] = buffer[i];
            }
            message_buffer[j] = '\0';
            for (i = after_start,j=0; i < after_end; i++,j++)
            {
                after_buffer[j] = buffer[i];
            }
            after_buffer[j] = '\0';
            // writes_nl_uart(buffer);
            // writes_nl_uart(message_buffer);
            // writes_nl_uart(after_buffer);
            int after = str2int(after_buffer);
            // write_int_uart(after,TRUE);
            // write_int_uart(message_start,TRUE);
            // write_int_uart(message_end,TRUE);
            // write_int_uart(after_start,TRUE);
            // write_int_uart(after_end,TRUE);
            add_timer(writes_nl_uart,message_buffer,after);
            break;
        }
        default:
            writes_uart("command not found: ");
            writes_uart(buffer);
            strcmp(buffer,"help");
            writes_uart("\r\n");
            break;
    }
    
    return;
}