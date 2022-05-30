#ifndef __UTILS_H_
#define __UTILS_H_
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
/* Function to calculate x raised to the power y in O(logn)*/
static inline int power(int x, unsigned int y)
{
    int temp;
    if( y == 0)
        return 1;
    temp = power(x, y/2);
    if (y%2 == 0)
        return temp*temp;
    else
        return x*temp*temp;
}
static inline unsigned int findPreviousPowerOf2(unsigned int n)
{
    unsigned int x=n;
    // do till only one bit is left
    while (x & (x - 1)) {
        
        //writehex_uart(x&(x-1));
        x = x & (x - 1);        // unset rightmost bit
    }
 
    // `n` is now a power of two (less than or equal to `n`)
    return x;
}
static inline unsigned int powOf2ToExponent(int n){
    int num = n;
    int exp = -1;
    while(num>0){
        num = num >> 1;
        ++ exp;
    }
    return exp;
}

#endif