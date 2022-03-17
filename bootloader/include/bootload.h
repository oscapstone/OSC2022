#define kernel_addr ((volatile unsigned int*)(0x80000))
#define relocate_addr ((volatile unsigned int*)(0x60000))
//extern unsigned long dtb_addr;
void bootload_image(unsigned long);
void relocate(unsigned int*);