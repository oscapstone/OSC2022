#include <test_fs.h>
#include <vfs.h>
#include <tmpfs.h>
#include <uart.h>

void fs_test1(){
    File *file_a = NULL;
    int err = vfs_open("/hello", 0, &file_a);
    if(err){
        uart_puts("[x] Failed to open file_a 1\n");
    }
    err = vfs_open("/hello", O_CREAT, &file_a);
    if(err){
        uart_puts("[x] Failed to open file_a 2\n");
    }
    vfs_close(file_a);

    File *file_b = NULL;
    err = vfs_open("/hello", 0, &file_b);
    if(err){
        uart_puts("[x] Failed to open file_b 1\n");
    }
    vfs_close(file_b);
}