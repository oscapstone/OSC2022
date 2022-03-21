#include "string.h"

void memcpy(char *s1, const char *s2, unsigned int len){
  for(int i = 0; i < len; i++)
    s1[i] = s2[i];
}

void memset(char *s1, const char c, unsigned int len){
  for(int i = 0; i < len; i++)
    s1[i] = c;
}

int strcmp(char *s1, char *s2){
    int i;
    for(i = 0; i < strlen(s1); i++){
        if(s1[i] != s2[i])
            return s1[i] - s2[i];
    }
    return s1[i] - s2[i];
}

unsigned int strlen(char *s){
    unsigned int len = 0;
    while(1){
        if(*(s + len) == '\0')
            break;
        len++;
    }
    return len;
}

void strrev(char *s){
    unsigned int len = strlen(s);
    char tmp;
    for(unsigned int i = 0; i < len/2; i++){
        tmp = s[i];
        s[i] = s[len-i-1];
        s[len-i-1] = tmp;
    }
}

// int to string
void itoa(int x, char *s){
    int i = 0, negative = 0, len = strlen(s);
    memset(s, 0, len);
    // handle 0 explicitly
    if(x == 0){
        s[i++] = '0';
        s[i] = '\0';
        return ;
    }
    // handle negative numbers
    if(x < 0){
        negative = 1;
        x = -x;
    }
    // process individual digits
    while(x != 0){
        s[i++] = x % 10 + '0';
        x = x / 10;
    }
    if(negative)
        s[i++] = '-';
    s[i] = '\0';
    strrev(s);
}

// unsigned int to hex (char *)
void uitohex(unsigned int x, char *s){
    int i = 0, rem = 0, len = strlen(s);
    memset(s, 0, len);
    // handle 0 explicitly
    if(x == 0){
        s[i++] = '0';
        s[i] = '\0';
        return ;
    }
    // process individual digits
    while(x != 0){
        rem = x % 16;
        if(rem >= 10)
            s[i++] = rem - 10 + 'A';
        else
            s[i++] = rem + '0';
        x = x / 16;
    }
    s[i] = '\0';
    strrev(s);
}