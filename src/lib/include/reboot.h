#ifndef __REBOOT__H__
#define __REBOOT__H__

void set(long addr, unsigned int value);
void reboot(int tick);
void cancel_reboot();

#endif