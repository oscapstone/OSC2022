#include "mini_uart.h"
#include "shell.h"
#include "utils.h"
#include "reboot.h"
#include "peripherals/mail_box.h"
#include "mail_box.h"
#include "cpio.h"
#include "timer.h"
#include "memory.h"
#include "allocator.h"
#include "printf.h"


int debug_mode = 0;
char buffer[MAX_BUFFER_SIZE];

void get_command();
void parse_command();
uint64_t test_page_malloc();
void test_page_free(uint64_t addr);

void get_board_revision();

void shell(void) {
    while (1) {
        uart_send_string("> ");
        get_command();
        parse_command();
    }
}

void get_command() {
    unsigned int index = 0;
    char recv;
    while (1) {
        recv = uart_recv();
        if (recv == '\r')
            continue;
        uart_send(recv);
        buffer[index++] = recv;
        index = index < MAX_BUFFER_SIZE ? index : MAX_BUFFER_SIZE - 1;
        if (recv == '\n') {
            buffer[index - 1] = '\0';
            break;
        }
    }
}

unsigned int debug_printf(char* fmt,...) {
   if (debug_mode) {
       char dst[100];
        __builtin_va_list args;
        __builtin_va_start(args,fmt);
        unsigned int ret=vsprintf(dst,fmt,args);
        uart_send_string(dst);
        return ret;
   }
   return 0;
}

void parse_command() {
    if (compare_string(buffer, "\0") == 0) {}
    else if (compare_string(buffer, "hello") == 0)
        uart_send_string("Hello World!\n");
    else if (compare_string(buffer, "reboot") == 0) {
        uart_send_string("rebooting ...\n");
        reboot(100);
        while (1) {}
    }
    else if (compare_string(buffer, "info") == 0) {
        get_board_revision();
        get_arm_memory();
    }
    else if (compare_string(buffer, "ls") == 0) {
        cpio_list();
    }
    else if (compare_string(buffer, "cat") == 0) {
        uart_send_string("Filename: \n");
        get_command();
        cpio_cat(buffer);
    }
    else if (compare_string(buffer, "load") == 0) {
        load_program();
    }
    else if (compare_string(buffer, "async_uart") == 0) {
        test_uart_async();
    }
    else if (compare_string(buffer, "test_timer") == 0) {
        test_timer();
    }
    else if (compare_string(buffer, "test_page1") == 0) {
        debug_mode = 1;
        const int test_size = 9;
        uint64_t page_addrs[test_size];
        for (int i = 0; i < test_size; ++i)
            page_addrs[i] = test_page_malloc();
        for (int i = 0; i < test_size; ++i)
            test_page_free(page_addrs[i]);
        debug_mode = 0;
    }
    else if (compare_string(buffer, "test_page2") == 0) {
        debug_mode = 1;
        const int test_size = 9;
        uint64_t page_addrs[test_size];
        for (int i = 0; i < test_size; ++i)
            page_addrs[i] = test_page_malloc();
        test_page_free(page_addrs[7]);
        test_page_free(page_addrs[8]);
        test_page_free(page_addrs[1]);
        test_page_free(page_addrs[4]);
        test_page_free(page_addrs[0]);
        test_page_free(page_addrs[5]);
        test_page_free(page_addrs[3]);
        test_page_free(page_addrs[2]);
        test_page_free(page_addrs[6]);
        debug_mode = 0;
    }
    else if (compare_string(buffer, "test_dyn") == 0) {
        debug_mode = 1;
        const int test_size = 10;
        int test_cases[] = {31, 2, 32, 63, 120, 256, 129, 155, 240, 250};
        void *addrs[test_size];
        for (int i = 0; i < test_size; ++i) {
            addrs[i] = kmalloc(test_cases[i]);
            print_slot_record(find_page(addrs[i]));
            uart_printf("\n");
        }
        for (int i = 0; i < test_size; ++i) {
            kfree(addrs[i]);
            print_slot_record(find_page(addrs[i]));
            uart_printf("\n");
        }
        debug_mode = 0;
    }
    else if (compare_string(buffer, "help") == 0) {
        uart_send_string("help               : print this help menu\n");
        uart_send_string("hello              : print Hello World!\n");
        uart_send_string("reboot             : reboot the device\n");
        uart_send_string("info               : print device info\n");
        uart_send_string("ls                 : print files in rootfs\n");
        uart_send_string("cat                : print file content\n");
        uart_send_string("load               : load user program\n");
        uart_send_string("async_uart         : test async uart\n");
        uart_send_string("test_timer         : test timer multiplexing\n");
        uart_send_string("test_page1         : test buddy system\n");
        uart_send_string("test_page2         : test buddy system\n");
        uart_send_string("test_dyn           : test dynamic allocator\n");
    }
    else
        uart_send_string("\rcommand not found!\r\n");
}

uint64_t test_page_malloc() {
    uart_printf("\n========= [Test Page Malloc] =========\n");
    uint64_t addr = page_malloc();
    uart_printf("return addr: 0x%x\n", addr);
    print_frame_array();
    print_frame_free_lists();
    uart_printf("======================================\n");
    return addr;
}

void test_page_free(uint64_t addr) {
    uart_printf("\n========== [Test Page Free] ==========\n");
    page_free(addr, 0);
    print_frame_array();
    print_frame_free_lists();
    uart_printf("======================================\n");
}