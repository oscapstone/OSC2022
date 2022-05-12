unsigned long mem_position;
void* simple_malloc(unsigned long size);
void init_buddy();
void* kmalloc(unsigned int size);
void kfree(void *ptr);
void test_malloc();