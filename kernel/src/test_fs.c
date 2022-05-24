#include <test_fs.h>
#include <vfs.h>
#include <tmpfs.h>
#include <uart.h>
#include <string.h>

void fs_test1(){
    uart_puts("--------------------TEST VFS_OPEN--------------------\n");
    File *file_a = NULL;
    int err = vfs_open("/hello", 0, &file_a);
    if(err) uart_puts("[x] Failed to open hello\n");

    err = vfs_open("/hello", O_CREAT, &file_a);
    if(err) uart_puts("[x] Failed to open file_a 2\n");
    vfs_close(file_a);

    File *file_b = NULL;
    err = vfs_open("/hello", 0, &file_b);
    if(err) uart_puts("[x] Failed to open file_b 1\n");
    vfs_close(file_b);
}

void fs_test2(){
    uart_puts("--------------------TEST VFS_READ & VFS_WRITE--------------------\n");
    File *a = NULL;
    int err = vfs_open("/hello", O_CREAT, &a);
    if(err) uart_puts("[x] Failed to open \"hello\"\n");
    // uart_puts(a->vnode->dentry->parent->name);

    File *b = NULL;
    err = vfs_open("/world", O_CREAT, &b);
    if(err) uart_puts("[x] Failed to open \"world\"\n");

    int len = vfs_write(a, "Hello ", 6);
    len = vfs_write(a, "Hello ", 6);
    if(len == -1) uart_puts("[x] Failed to write \"Hello\"\n");
    len = vfs_write(b, "World\n", 6);

    vfs_close(a);
    vfs_close(b);

    err = vfs_open("/hello", O_CREAT, &b);
    if(err) uart_puts("[x] Failed to open \"hello\"\n");
    err = vfs_open("/world", O_CREAT, &a);
    if(err) uart_puts("[x] Failed to open \"world\"\n");
    
    char buf[300];
    int sz;
    sz = vfs_read(b, buf, 100);
    sz += vfs_read(a, buf + sz, 100);
    len = vfs_read(b, buf, 100);
    if(len == -1) uart_puts("[x] Failed to read \"Hello\"\n");
    buf[sz] = '\0';
    uart_puts(buf);
    // printf("%s\n", buf); // should be Hello World!
}