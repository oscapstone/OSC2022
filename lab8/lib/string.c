#include "string.h"
#include "stdint.h"

int strncmp(const char *s1, const char *s2, uint32_t len) {
  for (int i = 0; i < len; i++) {
    if (s1[i] > s2[i]) return 1;
    else if (s1[i] < s2[i]) return -1;
    else if (s1[i] == '\0' || s2[i] == '\0') break;
  }
  return 0;
}

void* memset(void *dst, char c, uint32_t len) {
  char* dup = (char*)dst;
  while (len--) {
    *dup++ = c;
  }
  return dst;
}


char* strncpy(char *dst, const char *src, uint32_t len) {
  for (uint32_t i = 0; i < len; i++) {
    dst[i] = src[i];
    if (src[i] == 0) break;
  }
  return dst;
}

void* memcpy(void *dst, const void *src, uint32_t len) {
  char *cdst = (char*)dst;
  char *csrc  =(char*)src;
  for (uint32_t i = 0; i < len; i++) {
    cdst[i] = csrc[i];
  }
  return dst;
}

uint32_t strlen(const char *s) {
  uint32_t l = 0;
  while (*s++) l++;
  return l;
}


char *strtok(char *str, const char *delim) {
  static char *s;
  if (str != 0) s = str;
  else if (s == 0) return 0;

  // find first char that is not delim
  char *itr = s;
  int delim_len = strlen(delim);
  while (*itr != 0) {
    int match = 0;
    for (int i = 0; i < delim_len; i++) {
      if (*itr == delim[i]) {
        *itr = '\0';
        itr++;
        match = 1;
        break;
      }
    }
    if (!match) break;
  }
  if (*itr == '\0') {
    s = 0;
    return 0;
  }
  s = itr;
  // find a delim and make the delim become '\0'.
  while (*itr != 0) {
    for (int i = 0; i < delim_len; i++) {
      if (*itr == delim[i]) {
        *itr = '\0';
        char *org = s;
        s = itr+1;
        return org;
      }
    }
    itr++;
  }

  char *org = s;
  s = 0;  
  return org;
}
