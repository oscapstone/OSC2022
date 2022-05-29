#include "system_call.h"

void fs_test() {
    char buffer_mkdir[10] = "test fs!\n";
    mkdir("../home/d");
    if (mount(0, "d", "tmpfs", 0, 0) == -1) {
        printf("[ERROR] mount fail\n");
        while (1) {}
    }
    int fd = open("/home/d/t.txt", O_CREAT);
    write(fd, buffer_mkdir, 10);
    close(fd);
    fd = open("./d/t.txt", O_CREAT);
    read(fd, buffer_mkdir, 10);
    close(fd);
    printf("%s\n", buffer_mkdir);

    exit();
}