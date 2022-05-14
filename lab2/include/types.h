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


#define offset_of(type, member) \
    ((uint64_t) &((type*)0)->member)

#define container_of(ptr, type, member) \
    ((type* )((char*)ptr - offset_of(type, member)))

#define ALIGN_UP(val, align) ((val + (align - 1)) & ~(align - 1))
#endif
