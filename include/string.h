#ifndef STRING_H
#define STRING_H

#define ENDL "\r\n"

void memcpy(char *s1, char *s2, unsigned int len);
void memset(char *s1, char c, unsigned int len);
unsigned int strlen(char *s);
int strcmp(char *s1, char *s2);
int strncmp(char *s1, char *s2, unsigned int n);
void strrev(char *s);
int atoi(char* s);
char* itoa(int x, char *str);
void uint_to_hex(unsigned int x, char *s);
unsigned int hex_to_uint(char *s, unsigned int size);
unsigned int BE_to_uint(void *ptr);

#endif