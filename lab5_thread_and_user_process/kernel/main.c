#include "uart.h"
#include "power.h"
#include "string.h"
#include "utils.h"
#include "address.h"
#include "cpio.h"
#include "type.h"
#include "memory.h"
#include "timer.h"
#include "interrupt.h"
#include "thread.h"
#include "syscall.h"


int main(void) {
    uart_init();
    uart_getc(); // flush init signal(0xE0)

    initMemoryPages();
    initThreads();
    uart_dem();
    uart_prefix();

    enable_interrupt();
    enable_uart_interrupt();
    enable_core_timer();
    set_core_timer_by_second(1);

    uint64 tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
    
        
    char command[256];
    int i = 0;
    char c;

    while(1) {
        c = uart_async_getc();

        if(c == '\n') { // send command
            uart_newline();
            command[i] = '\0';
            i = 0;

            if(strcmp(command, "help")) {
                uart_puts("# help\n"
                          "help:\t\t print this help menu\n"
                          "thread:\t\t run test thead fn\n"
                          "fork:\t\t run test fork fn\n"
                          "ls:\t\t list user program\n"
                          "program:\t run video player\n"
                          "reboot:\t\t reboot the device");
            }
            else if(strcmp(command, "thread")) {
                for(int i = 0; i < 3; ++i) { // N should > 2
                    uart_puts("Create thead: "); uart_num(i); uart_newline();
                    createThread(testThread);
                }
                idle();
            }
            else if(strcmp(command, "fork")) {
                createThread(fork_test);
                idle();
            }
            else if(strcmp(command, "ls")) {
                extract_cpio(INITRAMFS_ADDR, 1, 0, 0x00);
            }
            else if(strcmp(command, "program")) {
                char* program_address = load_user_program(INITRAMFS_ADDR, NULL, "syscall.img");
                uint64 program_size = get_program_size(INITRAMFS_ADDR, "syscall.img");
                uart_puts("Start run user program at "); uart_hex(program_address); uart_puts(", with size = ");
                uart_num(program_size); uart_newline();
                execThread(program_address, program_size);
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
        else if(c == NULL) {
            continue;
        }
        else { // type word
            uart_putc(c); // remote echo
            command[i++] = c;
        }
    }

    return 0;
}