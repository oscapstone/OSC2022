#include "string.h"

#include "compiler.h"

// https://stackoverflow.com/a/34873763/13177700
int strcmp(const char* s1, const char* s2) {
    const uchar* p1 = (const uchar*)s1;
    const uchar* p2 = (const uchar*)s2;
    while (*p1 && *p1 == *p2) ++p1, ++p2;
    return (*p1 > *p2) - (*p2 > *p1);
}

int strncmp(const char* s1, const char* s2, int len) {
    const uchar* p1 = (const uchar*)s1;
    const uchar* p2 = (const uchar*)s2;
    while (len--) {
        if (*p1 != *p2)
            return (*p1 > *p2) - (*p2 > *p1);
        p1++, p2++;
    }
    return 0;
}

uint strlen(const char* s) {
    uint len = 0;
    while (*s++)
        ++len;
    return len;
}

uint isspace(char c) {
    if (unlikely(c == ' ' || c == '\t'))
        return 1;
    return 0;
}

// https://stackoverflow.com/a/122616/
void trim(char* str) {
    int i;
    int begin = 0;
    uint end = strlen(str) - 1;
    if (end == -1)
        return;

    while (isspace((uchar)str[begin]))
        begin++;

    while ((end >= begin) && isspace((uchar)str[end]))
        end--;

    // Shift all characters back to the start of the string array.
    for (i = begin; i <= end; i++)
        str[i - begin] = str[i];

    str[i - begin] = '\0';  // Null terminate string.
}

void memset(char* s, char value, uint size) {
    while (size--)
        *s++ = value;
}

void strins(char* s, char c, int pos) {
    uint len = strlen(s);
    for (uint i = len; i > pos; i--) {
        s[i] = s[i - 1];
    }
    s[pos] = c;
}

void strdel(char* s, int pos) {
    uint len = strlen(s) - 1;
    for (uint i = pos; i < len; i++) {
        s[i] = s[i + 1];
    }
    s[len] = 0;
}