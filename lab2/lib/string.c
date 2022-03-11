#include "printf.h"

int strlen(char *s){
    int i = 0;
    while(s[i] != '\0'){
        i++;
    }
    return i;
}

void append_str(char *s, char ch){
    int i = strlen(s);
    *(s+i) = ch;
    *(s+i+1) = 0;
}

void pop_str(char *s){
    int i = strlen(s);
    s[i-1] = 0;
}

int strcmp(const char *X, const char *Y)
{
    while (*X)
    {
        if (*X != *Y) {
            break;
        }
        X++;
        Y++;
    }
    return *(const unsigned char*)X - *(const unsigned char*)Y;
}

int myAtoi(char* str){

  int res = 0;

  for (int i = 0; str[i] != '\0'; ++i)
    res = res * 10 + str[i] - '0';

  return res;
}
