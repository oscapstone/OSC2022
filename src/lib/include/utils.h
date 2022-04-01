#ifndef __UTILS__H__
#define __UTILS__H__

typedef unsigned int size_t;

unsigned long align_by_4(unsigned long value);
void* simple_malloc(size_t size);
void unsignedlonglongToStrHex(unsigned long long num, char *buf);

#endif