#include "lib/string.h"
#include <stdio.h>

int main(void){
    char *name = "";
    printf("%s, len=%lu\n", name, strlen(name));
    
    name = "test";
    printf("%s, len=%lu\n", name, strlen(name));
    
    name = "xiaobye";
    printf("%s, len=%lu\n", name, strlen(name));
    return 0;
}
