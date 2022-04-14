#include "uart.h"
#include "power.h"
#include "string.h"
#include "utils.h"
#include "address.h"
#include "cpio.h"
#include "type.h"
#include "memory.h"

extern char* _user_program;  // linker script symbol

int main(void) {
    uart_init();
    uart_getc(); // flush init signal(0xE0)
    
    initMemoryPages();
    uart_dem();
    uart_prefix();
    
    
    char command[256];
    int i = 0;
    char c;
    page_t* pages[10];
    char* messages[10];

    while(1) {
        c = uart_getc();

        if(c == '\n') { // send command
            uart_newline();
            command[i] = '\0';
            i = 0;

            if(strcmp(command, "help")) {
                uart_puts("# help\n"
                          "help:\t\t\t print this help menu\n"
                          "newPage [N][T]: \t allocate a new page memory at N order [index T]\n"
                          "freePage [T]: \t\t free page memory at [index T]\n"
                          "allPage: \t\t show all page state\n"
                          "malloc [N][T]: \t\t malloc N bytes memory in index T\n"
                          "free [T]: \t\t\t free memory in index T\n"
                          "reboot:\t\t\t reboot the device");
            }
            else if(startwith(command, "newPage ")) {
                char* N = find_token(command, ' ');
                char* T = find_token(N, ' ');
                int order = str2num(N, 1);
                int idx = str2num(T, 1);
                pages[idx] = getFreePageAtOrder(order);
                uart_puts("Store page at arrry-"); uart_num(idx); uart_newline();
            }
            else if(startwith(command, "freePage ")) {
                char* T = find_token(command, ' ');
                int idx = str2num(T, 1);
                freePage(pages[idx]);
                uart_puts("Free page at arrry-"); uart_num(idx); uart_newline();
            }
            else if(strcmp(command, "allPage")) {
                loggingAllPageState();
            }
            else if(startwith(command, "malloc ")) {
                char* N = find_token(command, ' ');
                char* T = find_token(N, ' ');
                *(T-1) = '\0';
                int size = str2num(N, strlen(N));
                int idx = str2num(T, 1);
                messages[idx] = (char*)malloc(size);
                uart_puts("Malloc memory at "); uart_hex(messages[idx]); 
                uart_puts(" with size "); uart_num(size);
                uart_puts(" at arrry-"); uart_num(idx); uart_newline();

                // uart_puts("Please enter content of memory: ");
                
                // size--;
                // for(int i = 0; i < size; i++) {
                //     c = uart_getc();
                //     uart_putc(c);
                //     messages[idx][i] = c;
                // }
                // messages[idx][size] = '\0';
                // uart_puts("\nEcho message: ");
                // uart_puts(messages[idx]);
                // uart_newline();
            }
            else if(startwith(command, "free ")) {
                char* T = find_token(command, ' ');
                int idx = str2num(T, 1);
                uart_puts("Free memory = "); uart_hex(messages[idx]);
                uart_puts(" at arrry-"); uart_num(idx); uart_newline();
                free(messages[idx]);
            }
            else if(strcmp(command, "reboot")) {
                uart_async_puts("Reboot the board\n");
                delay_ms(1000); // wait for message transmit 
                reset();
            }
            else {
                uart_puts(command);
                uart_puts(" is not a valid command\n");
            }

            uart_dem();
            uart_prefix();
        }
        else if(c == 0x08 || c == 0x7F) {
            i--;
            uart_puts("\b \b");
            continue;
        }
        else { // type word
            uart_putc(c); // remote echo
            command[i++] = c;
        }
    }

    return 0;
}