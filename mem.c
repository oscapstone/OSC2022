#include "mem.h"
#include "utils.h"

char mem[65536];
unsigned long mem_position;
unsigned int mem_count = 0;

void* simple_malloc(unsigned int size) {
    mem_position = (unsigned long)&mem[mem_count];
    mem_count += size;
    return ((void*)mem_position);
}

void test_malloc() {
    char* string1 = simple_malloc(6);
    string1[0] = 'q';
    delay(1);
    string1[1] = 'w';
    delay(1);
    string1[2] = 'e';
    delay(1);
    string1[3] = 'r';
    delay(1);
    string1[4] = 't';

    char* string2 = simple_malloc(6);

    string2[0] = '1';
    delay(1);
    string2[1] = '2';
    delay(1);
    string2[2] = '3';
    delay(1);
    string2[3] = '4';
    delay(1);
    string2[4] = '5';
    delay(1);
    string2[4] = '\0';
}