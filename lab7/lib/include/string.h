#include "stdint.h"

typedef uint64_t size_t;

void memcpy(void* dest, const void* src, size_t len);
int strlen(char *s);
int atoi(char* str);
void append_str(char *s, char ch);
void pop_str(char *s);
char* strcpy(char* dest, const char* src);
int strcmp(const char *X, const char *Y);
int myHex2Int(char* str);
char *strtok(char *str, char *delimiter);
int spilt_strings(char** str_arr, char* str, char* deli);
