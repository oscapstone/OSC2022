#ifndef STRING
#define STRING

int strcmp (const char* str1, const char* str2);
void strcpy(char* dst, const char* src);
int startwith (const char* str1, const char* str2);
int strlen (const char* str1);
unsigned int str2num(char* str, int len);
char* find_token(const char* str, char sep);

#endif