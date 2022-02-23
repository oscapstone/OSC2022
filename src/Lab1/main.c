#include "uart.h"

int main() {

    uart_init();
    uart_flush();

    char start_prompt[8] = {'A', 'H', 'O', 'Y', '!', '\n', '\r', '\0'};
    char data;

    for (int i = 0; i < 7; i++) uart_put(start_prompt[i]);

    while (1) {
        data = uart_get();
        uart_put(data);
        if (data == '\n') uart_put('\r');
    }

    return 0;
}