#include "string.h"
#include "mini_uart.h"
int
strcmp (const char *p1, const char *p2)
{
  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;
  do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0')
        return c1 - c2;
    }
  while (c1 == c2);

  return c1 - c2;
}

int strncmp(const char *s1, const char *s2, unsigned int n)
{
  register unsigned char u1, u2;
  while (n-- > 0)
  {
    u1 = (unsigned char) *s1++;
    u2 = (unsigned char) *s2++;
    if (u1 != u2)
      return u1 - u2;
    if (u1 == '\0')
      return 0;
  }
  return 0;
}


int strlen(char *s){
    unsigned int count = 0;
    char *tmps = s;
    while(*tmps!='\0')
    {
        count++;
        tmps++;
    }
    return count;
}

unsigned int str2int(char *s){
  unsigned int n=0;
  //busy_wait_writes("Enter str2int",TRUE);
  for (int i = 0; i < strlen(s); i++)
  {
    if(s[i]<'0' || s[i]>'9')
      return n;
    //busy_wait_writes("Lopping str2int",TRUE);
    n*=10;
    n+=s[i]-'0';
  }
  //busy_wait_writes("Return str2int",TRUE);
  return n;
}