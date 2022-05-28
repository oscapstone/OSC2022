#include "mini_uart.h"
#include "shell.h"
#include "exception.h"
#include "timer.h"
#include "memory.h"
#include "allocator.h"
#include "task.h"
#include "vfs.h"
#include "tmpfs.h"


void kernel_main(void) {
	uart_init();
	memory_init();
	init_reserve();
	init_timer();
	enable_interrupt();
	create_root_thread();

	// setup tmpfs
	filesystem* fs = kmalloc(sizeof(filesystem));
	register_filesystem(fs, "tmpfs");
	fs->setup_mount(fs, rootfs);
    //tmpfs_dump(rootfs->root, 0);
	
	shell();
}
