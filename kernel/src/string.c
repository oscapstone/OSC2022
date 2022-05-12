/* Copyright (C) 1991-2022 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <string.h>
#include <uart.h>


/* Compare S1 and S2, returning less than, equal to or
   greater than zero if S1 is lexicographically less than,
   equal to or greater than S2.  */
int strcmp(const char *p1, const char *p2)
{
  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;

  do{
    c1 = (unsigned char) *s1++;
    c2 = (unsigned char) *s2++;
    if (c1 == '\0')
    return c1 - c2;
  }
  while (c1 == c2);

  return c1 - c2;
}

int strncmp(const char *p1, const char *p2, unsigned int n)
{
  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;

  if (n <= 0) return 0;
  while(n--){
    c1 = (unsigned char) *s1++;
    c2 = (unsigned char) *s2++;
    if (c1 == '\0' || c1 != c2)
      return c1 - c2;
  }

  return c1 - c2;
}

/* string length */
unsigned int strlen(const char buf[MAX_SIZE]){
  unsigned int len;
  for(len = 0; len < MAX_SIZE; len++){
    if(buf[len] == '\0') return(len);
  }
  return 0;
}

/* reverse string */
void reverse_string(char buf[MAX_SIZE]){
  unsigned int len = strlen(buf);
  char tmp;
  for(unsigned int i = 0; i < len/2; i++){
    tmp = buf[i];
    buf[i] = buf[len-i-1];
    buf[len-i-1] = tmp;
  }
}

/* int to string */
void itoa(char buf[MAX_SIZE], int num){
  int i = 0;
  int sign = 0;
  if(num < 0){
    sign = 1;
    num = -num;
  }
  do{
    buf[i++] = num % 10 + '0';
  }while((num /= 10) > 0);
  if(sign) buf[i++] = '-';
  buf[i] = '\0';
  reverse_string(buf);
}

/* uint to string */
void uitoa(char buf[MAX_SIZE], unsigned int num){
  unsigned int i = 0;
  do{
    buf[i++] = num % 10 + '0';
  }while((num /= 10) > 0);
  buf[i] = '\0';
  reverse_string(buf);
}

/* uint to hex string */
void uitohex(char buf[MAX_SIZE], unsigned int d){
  unsigned int i = 0;
  do{
    buf[i] = d % 16;
    if(buf[i] < 10) buf[i] += '0';
    else buf[i] += 'a' - 10;
    i++;
  }while((d /= 16) > 0);
  buf[i] = '\0';
  reverse_string(buf);
}

void print_string(enum print_type type, char *text , unsigned long long num, int println){
  char buf[20];
  switch(type){
    case UITOHEX:
      uart_puts(text);
      uitohex(buf, (unsigned int)num);
      uart_puts(buf);
      break;
    case UITOA:
      uart_puts(text);
      uitoa(buf, (unsigned int)num);
      uart_puts(buf);
      break;
    case ITOA:
      uart_puts(text);
      itoa(buf, (int)num);
      uart_puts(buf);
      break;
  }
  if(println) uart_puts("\n");
}

/* array to int */
int atoi(const char buf[MAX_SIZE]){
  int num = 0;
  int sign = 1;
  if(buf[0] == '-'){
    sign = -1;
    buf++;
  }
  for(unsigned int i = 0; i < strlen(buf); i++){
    num = num * 10 + (buf[i] - '0');
  }
  return num * sign;
}

/* array to uint */
unsigned int atoui(const char buf[MAX_SIZE]){
  unsigned int num = 0;
  for(unsigned int i = 0; i < strlen(buf); i++){
    num = num * 10 + (buf[i] - '0');
  }
  return num;
}

/* hex string to unsigned int */
unsigned int hextoui(const char buf[MAX_SIZE], unsigned int size){
  unsigned int num = 0;
  for(unsigned int i = 0; i < size; i++){
    if(buf[i] >= '0' && buf[i] <= '9'){
      num = num * 16 + (buf[i] - '0');
    }
    else if(buf[i] >= 'A' && buf[i] <= 'F'){
      num = num * 16 + (buf[i] - 'A' + 10);
    }
    else if(buf[i] >= 'a' && buf[i] <= 'f'){
      num = num * 16 + (buf[i] - 'a' + 10);
    }
  }
  return num;
}

void memcpy(char *d, const char *s, unsigned int len){
  for(unsigned int i = 0; i < len; i++){
    d[i] = s[i];
  }
}


void memset(char *d, const char s, unsigned int len){
  for(unsigned int i = 0; i < len; i++){
    d[i] = s;
  }
}

void strcpy(char *d, const char *s){
  unsigned int len = strlen(s);
  memcpy(d, s, len);
  d[len] = '\0';
}


void strcat(char *d, const char *s){
  unsigned int len = strlen(d);
  unsigned int slen = strlen(s);
  memcpy(d + len, s, slen);
  d[len + slen] = '\0';
}

char *strchr(const char *str, int c){
  for(unsigned int i = 0; i < strlen(str); i++){
    if(str[i] == c) return (char *)(str + i);
  }
  return 0;
}