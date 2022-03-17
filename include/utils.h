#ifndef uint32_t
    #define uint32_t unsigned int
#endif
#ifndef size_t
    #define size_t unsigned long
#endif
#ifndef uint64_t
    #define uint64_t unsigned long long
#endif

uint32_t big2little(uint32_t);
unsigned long align_up(unsigned long n, unsigned long align);