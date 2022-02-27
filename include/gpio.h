#ifndef GPIO_H
#define GPIO_H

#include "bcm2357.h"




#define _GPFSEL0_OFF            0x00200000
#define _GPFSEL1_OFF            0x00200004
#define _GPFSEL2_OFF            0x00200008
#define _GPFSEL3_OFF            0x0020000C


#define _GPPUD_OFF              0x00200094
#define _GPPUDCLK0_OFF          0x00200098
#define _GPPUDCLK1_OFF          0x0020009c


#define GPFSEL0                 _virt_addr(_GPFSEL0_OFF     )
#define GPFSEL1                 _virt_addr(_GPFSEL1_OFF     )
#define GPFSEL2                 _virt_addr(_GPFSEL2_OFF     )
#define GPFSEL3                 _virt_addr(_GPFSEL3_OFF     )

#define GPPUD                   _virt_addr(_GPPUD_OFF       )
#define GPPUDCLK0               _virt_addr(_GPPUDCLK0_OFF   )
#define GPPUDCLK1               _virt_addr(_GPPUDCLK1_OFF   )


#endif