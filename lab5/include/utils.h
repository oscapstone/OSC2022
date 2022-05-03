#ifndef UTILS_H
#define UTILS_H

void delay(unsigned long);
void put32(unsigned long, unsigned int);
unsigned int get32(unsigned long);
int get_el(void);
int hex_to_int(char *p, int len);

#endif /* UTILS_H */