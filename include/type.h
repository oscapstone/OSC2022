#ifndef TYPE
#define TYPE

#define NULL 0x00000000
#define true 1
#define false 0

typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef int int32;
typedef long long int64;
typedef unsigned char bool;
typedef int state_t;
typedef unsigned long long size_t;

#define IDLE 1
#define USED 0
#define DEAD -1
#define NEW_BORN 2

#define FREE 1
#define BUSY 0
#define EMPTY -1

#endif