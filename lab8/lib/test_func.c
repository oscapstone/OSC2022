

#include "syscall.h"
#include "string.h"
#include "user_lib.h"

void delay(int times) {
    while(times--) {
        asm volatile("nop");
    }
}

void fork_test() {
    printf("\nFork Test, pid %d\n", getpid());
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                delay(1000000);
                ++cnt;
            }
        }
        exit();
    }
    else {
        printf("parent here, pid %d, child %d\n", getpid(), ret);
    }
    exit(); 
}

void fs_test() {
    char buf[100];
    mkdir("mnt", 0);
    int fd = open("/mnt/a.txt", O_CREAT);
    write(fd, "Hi", 2);
    close(fd);
    chdir("mnt");
    fd = open("./a.txt", 0);
    if (fd < 0) {
        printf("error1\n");
        return;
    }
    read(fd, buf, 2);
    if (strncmp(buf, "Hi", 2) != 0) {
        printf("error2\n");
        return;
    }
    close(fd);

    chdir("..");
    mount("", "mnt", "tmpfs", 0, 0);
    mount("", "mnt", "tmpfs", 0, 0);
    fd = open("mnt/a.txt", 0);
    if (fd > 0) {
        printf("error3\n");
        return;
    }
}

void fat_test() {
    int fd = open("/boot/FAT_11.TXT", 0);
    int size = 600;
    char buf[size+1];
    for(int i=0 ; i<size ; i++)
        buf[i] = 'a' + (i%26);
    buf[size] = '\0';
    printf("%d\n", write(fd, buf, size));
    fd = open("/boot/FAT_11.TXT", 0);
    printf("%d\n", read(fd, buf, size));
    printf("%s\n", buf);
}

void fs_uart_test() {
    int fd = open("/dev/uart", 0);
    write(fd, "Hi", 2);
    write(fd, "Hi l\n", 5);
}

void initramfs_test() {
    char buf[8];
    int fd = open("/initramfs/dir1/file3.txt", 0);
    read(fd, buf, 100);
    printf("%s\n", buf);
    close(fd);
    fd = open("/initramfs/file1", 0);
    printf("%d\n", lseek64(fd, 0, SEEK_END));
    read(fd, buf, 100);
    printf("%s\n", buf);
    close(fd);
}