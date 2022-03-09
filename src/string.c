#include "string.h"
#include "type.h"



int strcmp(char* _str1, char* _str2) {
    char* cur1;
    char* cur2;
    cur1 = _str1;
    cur2 = _str2;
    while(*cur1 != '\0' && *cur2 != '\0' && (*cur1 == *cur2)) {
        cur1++;
        cur2++;
    }
    return (int32_t)(*cur1 - *cur2);
}



void* memset(void* ptr, int c, size_t size) {
    char* p = (char*)ptr;
    while(size--) { *p++ = c; }
    return p;
}

void* memcpy(void* dst, void* src, size_t size) {

    char* dst_cur = (char*) dst;
    char* src_cur = (char*) src;
    for(int i=0;i<size;i++) {
        *dst_cur++ = *src_cur++;
    }
    return dst_cur;
}