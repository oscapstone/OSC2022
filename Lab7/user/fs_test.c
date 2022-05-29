#include "system_call.h"

void fs_test() {
    char buffer_mkdir[10] = "test fs!\n";
    mkdir("/dir3-1/dir3-2");
    if (mount(0, "/dir3-1/dir3-2", "tmpfs", 0, 0) == -1) {
        printf("[ERROR] mount fail\n");
        while (1) {}
    }
    int fd = open("/dir3-1/dir3-2/test.txt", O_CREAT);
    write(fd, buffer_mkdir, 10);
    close(fd);
    fd = open("/dir3-1/dir3-2/test.txt", O_CREAT);
    read(fd, buffer_mkdir, 10);
    close(fd);
    printf("%s\n", buffer_mkdir);

    exit();
}