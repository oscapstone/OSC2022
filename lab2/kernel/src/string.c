#include "string.h"

int strcmp(char *s1, char *s2){
    for(int i = 0; ; i++) {
        if(s1[i] != s2[i]) {
            return 0;
        }
        else if(s1[i] == '\0' && s2[i] == '\0') {
            return 1;
        }
    }
}

int strncmp(char *s1, char *s2, unsigned long n){
    for(int i = 0; i < n; i++) {
        if(s1[i] != s2[i]) {
            return 0;
        }
        else if(s1[i] == '\0' && s2[i] == '\0') {
            return 1;
        }
    }
    return 1;
}

unsigned long long strlen(const char *str)
{
  unsigned long long count = 0;
  while((unsigned char)*str++)count++;
  return count;
}