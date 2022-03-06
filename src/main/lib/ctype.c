#include "ctype.h"


int is_digit(char c) {


    if(c >= 0x30 && c <= 0x39)
        return 1;
    return 0;

}



int is_alpha(char c) {

    if((c >= 0x61 && c <= 0x7a) || (c >= 0x41 && c <= 0x5a)) {
        return 1;
    }
    return 0;
}


int is_upper(char c) {

    if(c >= 0x41 && c <= 0x5a)
        return 1;
    return 0;

}