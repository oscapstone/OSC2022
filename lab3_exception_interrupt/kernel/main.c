#include "uart.h"
#include "power.h"
#include "string.h"
#include "utils.h"
#include "address.h"
#include "cpio.h"
#include "type.h"
#include "timer.h"
#include "interrupt.h"

extern char* _user_program;  // linker script symbol

int main(void) {
    uart_init();
    uart_getc(); // flush init signal(0xE0)
    uart_dem();
    uart_prefix();
    enable_interrupt();
    enable_uart_interrupt();
    enable_core_timer();
    set_core_timer_by_second(1); // enable timer multiplexing
    
    char command[256];
    int i = 0;
    char c;

    while(1) {
        c = uart_async_getc();

        if(c == '\n') { // send command
            uart_async_newline();
            command[i] = '\0';
            i = 0;

            if(strcmp(command, "help")) {
                uart_async_puts("# help\n"
                                "help:\t\t\t print this help menu\n"
                                "except:\t\t\t raise exception EL1\n"
                                "setTimeout [M][S]:\t set timer event\n"
                                "program:\t\t run user program at EL0\n"
                                "loop:\t\t\t add fake interrupt handle (loop)\n"
                                "reboot:\t\t\t reboot the device");
            }
            else if(strcmp(command, "except")) {
                uart_async_puts("Raise exception EL1\n");
                raise_exc();
            }
            else if(startwith(command, "setTimeout ")) {
                char* message = find_token(command, ' ');
                char* second_string = find_token(message, ' ');

                *(second_string - 1) = '\0'; // convert message last char from space to \0
                unsigned long long tick = get_current_tick();
                unsigned long long second = str2num(second_string, strlen(second_string));
                uart_async_puts("Add new timer, Current time(s): "); uart_async_num(tick2second(tick)); uart_async_puts(", ");
                uart_async_puts("Message: "); uart_async_puts(message); uart_async_puts(", ");
                uart_async_puts("Second: "); uart_async_puts(second_string); uart_async_puts("\n");
                
                add_timer(show_message, second2tick(second), message);
            }
            else if(strcmp(command, "loop")) {
                uart_async_puts("Run fake interrupt (loop)\n");
                add_interrupt(wait_loop, WAIT_INTERRUPT_PRIORITY);
            } 
            else if(strcmp(command, "program")) {
                uart_async_puts("Run user program at EL0\n");
                char* program_address = load_user_program(INITRAMFS_ADDR, _user_program, "user_program");
                uart_async_puts("Start run user program\n");
                
                add_timer(increment_timeout_2_seconds, second2tick(1), NULL);
                el1_to_el0(program_address, _user_program);
            }
            else if(strcmp(command, "reboot")) {
                uart_async_puts("Reboot the board\n");
                delay_ms(1000); // wait for message transmit 
                reset();
            }
            else {
                uart_async_puts(command);
                uart_async_puts(" is not a valid command\n");
            }

            uart_async_dem();
            uart_async_prefix();
        }
        else if(c == NULL) {
            continue;
        }
        else if(c == 0x08 || c == 0x7F) {
            i--;
            uart_async_puts("\b \b");
            continue;
        }
        else { // type word
            uart_async_putc(c); // remote echo
            command[i++] = c;
        }
    }

    return 0;
}