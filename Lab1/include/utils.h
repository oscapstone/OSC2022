#ifndef	_BOOT_H
#define	_BOOT_H

// unsigned long: 64 bits
// unsigned int: 32 bits
extern void delay ( unsigned long);
extern void put32 ( unsigned long, unsigned int );
extern unsigned int get32 ( unsigned long );

#endif  /*_BOOT_H */