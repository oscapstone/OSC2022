#ifndef	_REBOOT_H
#define	_REBOOT_H

void set(long addr, unsigned int value);
void reboot(int tick);
void cancel_reboot();

#endif