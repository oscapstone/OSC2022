#ifndef uint32_t
    #define uint32_t unsigned int
#endif
#ifndef size_t
    #define size_t unsigned long
#endif
#ifndef uint64_t
    #define uint64_t unsigned long long
#endif

#define TRUE  (1==1)
#define FALSE (!TRUE)
#define null 0
#define nullptr ((void *)0)
typedef int bool;



uint32_t big2little(uint32_t);
unsigned long align_up(unsigned long n, unsigned long align);