#ifndef	_P_BASE_H
#define	_P_BASE_H

// There is a VideoCore/ARM MMU translating physical addresses to bus addresses.
// The MMU maps physical address 0x3f000000 to bus address 0x7e000000.

#define MMIO_BASE 0x3F000000

#endif  /*_P_BASE_H */