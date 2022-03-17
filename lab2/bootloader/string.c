#include "string.h"
#include "math.h"

int strcmp ( char * s1, char * s2 )
{
    int i;

    for (i = 0; i < strlen(s1); i ++)
    {
        if ( s1[i] != s2[i])
        {
            return s1[i] - s2[i];
        }
    }

    return  s1[i] - s2[i];
}

void strset (char * s1, int c, int size )
{
    int i;

    for ( i = 0; i < size; i ++)
        s1[i] = c;
}

int strlen ( char * s )
{
    int i = 0;
    while ( 1 )
    {
        if ( *(s+i) == '\0' )
            break;
        i++;
    }

    return i;
}

// https://www.geeksforgeeks.org/convert-floating-point-number-string/
// void itoa (int x, char str[], int d) 
// { 
//     int i = 0; 
//     while (x) { 
//         str[i++] = (x % 10) + '0'; 
//         x = x / 10; 
//     } 
  
//     // If number of digits required is more, then 
//     // add 0s at the beginning 
//     while (i < d) 
//         str[i++] = '0'; 
    
//     str[i] = '\0'; 
//     reverse(str); 
// } 

 /* itoa:  convert n to characters in s */
 void itoa(int n, char s[])
 {
     int i, sign;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = (n % 16 < 10) ? (n % 16 + '0') : n % 16 - 10 + 'a';   /* get next digit */
     } while ((n /= 16) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }

 /* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

 unsigned long hextoint(char* addr, const int size){
    unsigned long res = 0;
    char c;
    for(int i = 0 ;i < size ;i++){
        res = res<<4;
        c = *(addr+i);
        if('0'<=c && c<='9'){
            res += c -'0';
        }else if('A'<=c && c<='F'){
            res += c - 'A' + 10;
        }else if('a'<=c && c<='f'){
            res += c - 'a' + 10;
        }
    }
    return res;
}

void strcpy(char *target, char *string){
    while(((*(target++))=(*(string++)))!='\0');
}
