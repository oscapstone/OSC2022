unsigned long mem_position;
void init_buddy();
void* kmalloc(unsigned int size);
void kfree(void *ptr);
void test_malloc();