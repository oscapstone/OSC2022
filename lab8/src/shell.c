#include "utils.h"
#include "uart.h"
#include "printf.h"
#include "memory.h"
#include "vfs.h"
#include "tmpfs.h"
#include "shell.h"
#include "sched.h"
#include "sys.h"
#include "sdhost.h"
#include "fat32.h"


void shell() {
    printf("\n\n _  _  ___ _____ _   _  ___  ___ ___ ___ \n");
    printf("| \\| |/ __|_   _| | | |/ _ \\/ __|   \\_ _|\n");
    printf("| .` | (__  | | | |_| | (_) \\__ \\ |) | | \n");
    printf("|_|\\_|\\___| |_|  \\___/ \\___/|___/___/___|\n\n");
    char input[1024];
    while (1) {
        uart_send('\r');
        uart_puts("# ");
        shell_input(input);
        if (strcmp(input, "test") == 0) {
            struct vnode *vnode;
            int fd = open("/dir", O_CREAT);
            int ret = write(fd, "abcdefghijklmnopqrstuvwxyz", 26);
            printf("%d %d\n", fd, ret);
            fd = open("/dir", 0);
            char buf[128];
            int rd = read(fd, buf, 10);
            printf("%d %d %s\n", fd, rd, buf);
            ret = vfs_lookup(".", &vnode);
            printf("Current directory: %s\n", ((struct tmpfs_internal *)vnode->internal)->name);


        } else if (strcmp(input, "m") == 0) {
            shell_input(input);
            int size = (int)cstr_to_ulong(input);
            void *ptr = malloc(size);
            printf("%x\n", ptr);
        } else if (strcmp(input, "d") == 0) {
            shell_input(input);
            void *ptr = (void *)(ulong)hex_to_int(input, 8);
            free(ptr);
        } else if (strcmp(input, "pm") == 0) {
            print_freelists();
            print_memory();
        } else if (!strcmp(input, "pwd")) {
            printf("%s\n", ((struct tmpfs_internal *)current_thread()->pwd->internal)->name);
        } else if (!strcmp(input, "sd")) {
            int idx;
            shell_input(input);
            char *buf = (char *)malloc(512);
            readblock(cstr_to_ulong(input), buf);
            for (int i = 0; i < 512; i++) {
                printf("%02x ", buf[i]);
                if (i % 16 == 15) printf("\n");
            }
        } else if (!strcmp(input, "meta")) {
            printf("fat_region_blk_idx %d\n", fat32_metadata.fat_region_blk_idx);
            printf("n_fat %d\n", fat32_metadata.n_fat);
            printf("sector_per_fat %d\n", fat32_metadata.sector_per_fat);
            printf("data_region_blk_idx %d\n", fat32_metadata.data_region_blk_idx);
            printf("first_cluster %d\n", fat32_metadata.first_cluster);
            printf("sector_per_cluster %d\n", fat32_metadata.sector_per_cluster);
        } else if (!strcmp(input, "vfs2")) {
            int ret;
            char *argv[] = {};
            if ((ret = fork()) == 0) {
                exec("vfs2.img", argv);
            }
            idle();
        } else {
            uart_puts("Error input!\n");
        }
    }

}

void shell_input(char *input) {
    int i = 0;
    char temp;
    while (1) {
        temp = uart_getc();
        if (temp == '\n') {
            uart_puts("\n");
            input[i] = '\0';
            break;
        } else
            uart_send(temp);

        input[i] = temp;
        i++;
    }
}