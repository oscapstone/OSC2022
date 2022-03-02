#ifndef STRING_H
#define STRING_H

void memcpy(char *s1, const char *s2, unsigned int len);
void memset(char *s1, const char c, unsigned int len);
int strcmp(char *s1, char *s2);
unsigned int strlen(char *s);
void strrev(char *s);
// int to string
void itoa(int x, char *s);
// unsigned int to hex (char *)
void uitohex(unsigned int x, char *s);

#endif