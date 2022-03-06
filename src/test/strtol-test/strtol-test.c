
#include <stdlib.h>
#include "strtol.h"
#include <stdio.h>


int main(void) {



    char* test[] = {
        "00000002",
        "12345678",
        "00002bfd",
        "deadbeef"
    };

    for(int i=0;i<4;i++) {

        uint32_t ans = _strtoll(test[i], NULL, 16);

        printf("%x\n", (unsigned int)ans);
    }


    return 0;


}