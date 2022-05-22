#ifndef _TYPES_H_
#define _TYPES_H_

typedef unsigned long int       uint64_t; 
typedef unsigned int            uint32_t;
typedef unsigned short int      uint16_t;
typedef unsigned char           uint8_t;

typedef long int                int64_t;
typedef int                     int32_t;
typedef short int               int16_t;
typedef signed char             int8_t;

typedef uint64_t                size_t;
typedef int64_t                 ssize_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

#define offset_of(type, member) \
    ((uint64_t) &((type*)0)->member)

#define container_of(ptr, type, member) \
    ((type* )((char*)ptr - offset_of(type, member)))

#define ALIGN_UP(val, align) (((val) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_DOWN(val, align) ((val) & ~((align) - 1))

#define bswap32(n) (((n & 0xff000000) >> 24) | \
                    ((n & 0x00ff0000) >>  8) | \
                    ((n & 0x0000ff00) <<  8) | \
                    ((n & 0x000000ff) << 24) )
#define bswap64(n) (((n & 0xff00000000000000) >> 56) | \
                    ((n & 0x00ff000000000000) >> 40) | \
                    ((n & 0x0000ff0000000000) >> 24) | \
                    ((n & 0x000000ff00000000) >>  8) | \
                    ((n & 0x00000000ff000000) <<  8) | \
                    ((n & 0x0000000000ff0000) <<  24)| \
                    ((n & 0x000000000000ff00) <<  40)| \
                    ((n & 0x00000000000000ff) <<  56)) 

#define min(x, y) ((x) < (y) ? (x) : (y)) 

#endif
