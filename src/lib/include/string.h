#ifndef __STRING__H__
#define __STRING__H__

int strcmp(const char *s1, const char *s2);
int strncmp(const char *cs, const char *ct, unsigned int count);
unsigned long hexStr2int(char *hex, int n);
unsigned int strlen(char *str);
unsigned long atou (char *s);
int exp(int num);
int ceiling(double x);
int pow(int base, int exponent);

#endif