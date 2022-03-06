#ifndef LIMITS_H
#define LIMITS_H



#define __LONG_MAX__    0x7fffffffffffffffL
#define __INT_MAX__     0x7fffffffL


#define LONG_MAX        __LONG_MAX__
#define LONG_MIN        (-LONG_MAX - 1L)


#define ULONG_MAX       (LONG_MAX * 2UL + 1UL)


#define INT_MAX         __INT_MAX__
#define INT_MIN         (-INT_MAX - 1)

#define UINT_MAX        (__INT_MAX__ * 2U + 1U)




#endif