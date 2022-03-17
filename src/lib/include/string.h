#ifndef __STRING__H__
#define __STRING__H__

int strcmp(const char *s1, const char *s2);
int strncmp(const char *cs, const char *ct, unsigned int count);
unsigned long hexStr2int(char *hex, int n);
unsigned int strlen(char *str);
#endif