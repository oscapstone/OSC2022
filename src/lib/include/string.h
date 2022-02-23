#ifndef __STRING__H__
#define __STRING__H__
#include "stddef.h"
void *memcpy(void *dst, void *src, size_t n);
void *memset(void *dst, int value, size_t n);
long strlen(char *s);
char *strcpy(char *dst, char *src);
char *strncpy(char *dst, char *src , size_t n);
char *strcat(char *dst, char *src);
char *strncat(char *dst, char *src, size_t n);
int strcmp(char *s1, char*s2);
int strncmp(char *s1, char *s2, size_t n);
#endif