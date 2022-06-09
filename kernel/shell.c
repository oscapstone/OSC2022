#include "shell.h"

char buf[0x1000];

struct cmd cmd_list[] = {
    {"help",      cmd_help,      "print this help menu"},
    {"hello",     cmd_hello,     "print Hello World!"},
    // {"reboot",    cmd_reboot,    "reboot the device"},
    {"revision",  cmd_revision,  "print board revision"},
    {"memory",    cmd_memory,    "print ARM memory base address and size"},
    {"ls",        cmd_ls,        "list directory contents"},
    {"cat",       cmd_cat,       "print file content"},
    {"dtb",       cmd_dtb,       "show device tree"},
    {"initramfs", cmd_initramfs, "show initramfs address"},
    // {"async",     cmd_async,     "test async print"},
    // {"prog",      cmd_prog,      "load a user program in the initramfs, and jump to it"},
    // {"sec2",      cmd_sec2,      "print the seconds after booting and set the next timeout to 2 seconds later."},
    // {"setTimeout",cmd_setTimeout,"prints message after seconds"},
    // {"testfoo",   cmd_foo,       "test thread"},
    // {"preempt",   cmd_preempt,   "test preemption"},
    // {"pageTest",  cmd_pageTest,  "test page frame allocator"},
    // {"chunkTest", cmd_chunkTest, "test small chunk allocator"},
    {"exec",      cmd_exec,      "run img file"},
};

void welcome_msg() {
    uart_printf("************************************************\r\n");
}

void read_cmd() {
    char c = 0;
    unsigned int idx = 0;
    while (1) {
        // read char
        uart_read(&c, 1);
        // c = uart_read_char_async();
        // handle buffer
        switch (c) {
            case '\r':
            case '\n':
                // echo char
                uart_write_char(c);
                buf[idx++] = '\0';
                return;
            // Backspaces
            case '\x7f':
                if (idx > 0) {
                    buf[--idx] = '\0';
                    uart_printf("\b \b");
                }
                break;
            // normal case
            default:
                // echo char
                uart_write_char(c);
                buf[idx++] = c;
                break;
        }
    }
}

void exec_cmd() {
    // null command
    if (!strlen(buf))
        return;
    // find index of delimiter
    unsigned int delimiter = 0;
    for (unsigned int i = 0; i < strlen(buf); i++) {
        if (buf[i] != ' ')
            delimiter++;
        else
            break;
    }
    // use delimiter to saperate cmd & param
    for (unsigned int i = 0; i < sizeof(cmd_list) / sizeof(struct cmd); i++) {
        // len of delimiter must larger than command
        unsigned int l = delimiter;
        if (l < strlen(cmd_list[i].name))
            l = strlen(cmd_list[i].name);
        // check command list
        if (!strncmp(buf, cmd_list[i].name, l)) {
            // buf + delimiter -> point to param
            char* param = buf + l;
            while (*param == ' ')
                param++;
            cmd_list[i].func(param);
            return;
        }
    }
    cmd_unknown();
}

void cmd_help(char* param) {
    unsigned int indent_size = 0;
    for (unsigned int i = 0; i < sizeof(cmd_list) / sizeof(struct cmd); i++) {
        uart_printf("%s", cmd_list[i].name);
        indent_size = 12 - strlen(cmd_list[i].name);
        while (indent_size--)
            uart_printf(" ");
        uart_printf(": %s\r\n", cmd_list[i].desc);
    }
}

void cmd_hello(char* param) {
    uart_printf("Hello World!\r\n");
}

void cmd_reboot(char* param) {
    reset(10);
}

void cmd_revision() {
    unsigned int board_revision;
    get_board_revision(&board_revision);
    uart_printf("Board Revision : 0x%x\r\n", board_revision);
}

void cmd_memory() {
    unsigned int arm_mem_base, arm_mem_size;
    get_arm_memory(&arm_mem_base, &arm_mem_size);
    uart_printf("ARM Memory Base Address : 0x%x\r\n", arm_mem_base);
    uart_printf("ARM Memory Size         : 0x%x\r\n", arm_mem_size);
}

void cmd_ls(char* param) {
    cpio_newc_parser(cpio_ls_callback, param);
}

void cmd_cat(char* param) {
    cpio_newc_parser(cpio_cat_callback, param);
}

void cmd_dtb(char* param) {
    dtb_parser(dtb_show_callback);
}

void cmd_initramfs() {
    uart_printf("Initramfs address: 0x%x\r\n", INITRD_ADDR);
}

void cmd_async() {
    uart_printf_async("%s, %s\r\n", "nlnlOuO", "nlnlSoFun");
}

void cmd_prog(char* param) {
    cpio_newc_parser(cpio_prog_callback, param);
}

void cmd_sec2() {
    add_timer(two_second_alert, 2, "", 0);
}

void cmd_setTimeout(char* param) {
    unsigned int idx = 0;
    char msg[1000];
    memset(msg, 0, 1000);
    while (*param != ' ')
        msg[idx++] = *param++;
    msg[idx++] = '\r';
    msg[idx++] = '\n';
    msg[idx++] = '\0';
    char *seconds = param + 1;
    add_timer(uart_write_string, atoi(seconds), msg, 0);
}

void cmd_preempt() {
    char tmp[0x100];
    for (int i = 0; i < 0x100; i++)
        tmp[i] = ('A' + (i % 26));
    add_timer(uart_write_string, 1, "Timer\r\n", 0);
    uart_write_string(tmp);
}

void cmd_pageTest() {
    page_allocator_test();
}

void cmd_chunkTest() {
    sc_test();
}

void cmd_exec(char* param) {
    cpio_newc_parser(cpio_exec_callback, param);
}

void cmd_unknown() {
    uart_printf("Err: command %s not found, try <help>\r\n", buf);
}

void foo() {
    for (int i = 0; i < 10; i++) {
        uart_printf("Thread id : %d, i : %d\r\n", curr_thread->pid, i);
        nop_delay(100000);
        schedule();
    }
    thread_exit();
}

void cmd_foo() {
    for (int i = 0; i < 3; i++) {
        thread_create(foo, 0x1000);
    }
    schedule();
}

void shell() {
    welcome_msg();
    // init thread scheduler
    init_thread_sched();
    // init timer
    timer_list_init();
    core_timer_enable();  
    
    while (1) {
        uart_printf("# ");
        read_cmd();
        uart_printf(ENDL);
        exec_cmd();
    }
}
