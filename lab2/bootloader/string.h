#ifndef STRING_H
#define STRING_H

int  strcmp     ( char * s1, char * s2 );
void strset     ( char * s1, int c, int size );
int  strlen     ( char * s );
void itoa(int n, char s[]);
void ftoa       ( float n, char* res, int afterpoint ); 
void reverse(char s[]);
unsigned long hextoint(char* addr, const int size);
void strcpy(char *target, char *string);

#endif