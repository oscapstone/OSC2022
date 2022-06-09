#include "shell.h"

#include "command.h"
#include "uart.h"
#include "exception.h"
#include "thread.h"
#include "mem.h"
#include "vfs.h"
#include "tmpfs.h"

extern struct mount* rootfs;

int parse(char input_char, int buffer_counter) {
    if ((input_char > 31 && input_char < 127) || input_char == 9) {
        buffer_counter++;
        uart_send(input_char);
    }
    else if (buffer_counter != 0 && (input_char == 8 || input_char == 127)) {
        // backspace
        buffer_counter--;
        uart_send(8);
        uart_send(' ');
        uart_send(8);
    }
    else if (input_char == '\n' || input_char == '\r') {
        // Enter
        uart_send(10);
    }

    return buffer_counter;
}

void shell() {
    int buffer_counter = 0;
    char buffer[MAX_BUFFER_LEN];

    int el;
    asm volatile("mrs x0, CurrentEL \n");
    asm volatile("lsr x0, x0, #2	\n");
    asm volatile("mov %0, x0	\n":"=r"(el):);
	printf("Exception level: %d \n", el);
    

    parse_command("mbox_board_revision");
    parse_command("mbox_arm_memory");
    // new line head
    uart_puts("# ");

    enable_current_interrupt(); // ------v
    init_schedule();

	register_filesystem("tmpfs");
    struct filesystem *tmp_fs = find_fs("tmpfs");
	tmp_fs->setup_mount(tmp_fs, rootfs);
    setup_uart_fs();

    // read input
    while (1) {
        #ifdef ASYNC_UART
        char input_char = async_uart_getc();
        #else
        char input_char = uart_getc();
        #endif
        buffer[buffer_counter] = input_char;
        buffer_counter = parse(input_char, buffer_counter);

        if (input_char == '\n') {
            // Enter
            buffer[buffer_counter] = 0;
            while (buffer_counter--) {
                if (buffer[buffer_counter] == ' ') {
                    buffer[buffer_counter] = 0;
                }
            }
            buffer_counter = 0;
            
            parse_command(buffer);
            uart_puts("# ");
        }
    }
}
