
// C code to demonstrate working of
// strtok
#include <stdio.h>
#include "string.h"
 
// Driver function
int main()
{
    // Declaration of string
    char gfg[100] = " Geeks - for - geeks - Contribute";
 
    // Declaration of delimiter
    const char s[4] = "-";
    char* tok;
 
    // Use of strtok
    // get first token
    tok = strtok(gfg, s);
 
    // Checks for delimiter
    while (tok != 0) {
        printf(" %s\n", tok);
 
        // Use of strtok
        // go through other tokens
        tok = strtok(0, s);
    }
 
    return (0);
}
