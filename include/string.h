#ifndef STRING
#define STRING

#define MAX_LEN 1025

int strcmp (const char* str1, const char* str2);
void strcpy(char* dst, const char* src);
int startwith (const char* str1, const char* str2);
int strlen (const char* str1);
unsigned int str2num(char* str, int len);
char* find_token(const char* str, char sep);
char *get_dirname(char* dst, const char* src);
char *get_rootname(char* dst, const char* src);
void get_abspath(char *abs_path, char *path, const char *curr_path);

#endif