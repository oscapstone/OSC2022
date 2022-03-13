#define kernel_addr ((volatile unsigned int*)(0x80000))
#define relocate_addr ((volatile unsigned int*)(0x60000))
void bootload_image();
void relocate(unsigned int*);