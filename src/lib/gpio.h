// =============================================================================
//  Program : gpio.h
//  Author  : YI-DE LEE
//  Date    : Feb/23/2022
// -----------------------------------------------------------------------------
//  Description:
//  
// -----------------------------------------------------------------------------
//  Revision information:
//
//  None.
// -----------------------------------------------------------------------------
//  License information:
//
//  This software is released under the BSD-3-Clause Licence,
//  see https://opensource.org/licenses/BSD-3-Clause for details.
//  In the following license statements, "software" refers to the
//  "source code" of the complete hardware/software system.
//
//  Copyright 2022,
//                    Embedded Intelligent Systems Lab (EISL)
//                    Deparment of Computer Science
//                    National Chiao Tung Uniersity
//                    Hsinchu, Taiwan.
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
// =============================================================================

/* ----------------------------------------------------------------------------------
| Address(PHY) | Field Name |              Description              | Bit Size | R/W |
| ------------ | ---------- |:-------------------------------------:| -------- |:---:|
| 0x7E200000   | GPFSEL0    |        GPIO Function Select 0         | 32       | R/W |
| 0x7E200004   | GPFSEL1    |        GPIO Function Select 1         | 32       | R/W |
| 0x7E200008   | GPFSEL2    |        GPIO Function Select 2         | 32       | R/W |
| 0x7E20000C   | GPFSEL3    |        GPIO Function Select 3         | 32       | R/W |
| 0x7E200010   | GPFSEL4    |        GPIO Function Select 4         | 32       | R/W |
| 0x7E200014   | GPFSEL5    |        GPIO Function Select 5         | 32       | R/W |
| 0x7E200018   | -          |               Reserved                | -        |  -  |
| 0x7E20001C   | GPSET0     |         GPIO Pin Output Set 0         | 32       |  W  |
| 0x7E200020   | GPSET1     |         GPIO Pin Output Set 1         | 32       |  W  |
| 0x7E200024   | -          |               Reserved                | -        |  -  |
| 0x7E200028   | GPCLR0     |        GPIO Pin Output Clear 0        | 32       |  W  |
| 0x7E20002C   | GPCLR1     |        GPIO Pin Output Clear 1        | 32       |  W  |
| 0x7E200030   | -          |               Reserved                | -        |  -  |
| 0x7E200034   | GPLEV0     |           GPIO Pin Level 0            | 32       |  R  |
| 0x7E200038   | GPLEV1     |           GPIO Pin Level 1            | 32       |  R  |
| 0x7E20003C   | -          |               Reserved                | -        |  -  |
| 0x7E200040   | GPEDS0     |    GPIO Pin Event Detect Status 0     | 32       | R/W |
| 0x7E200044   | GPEDS1     |    GPIO Pin Event Detect Status 1     | 32       | R/W |
| 0x7E200048   | -          |               Reserved                | -        |  -  |
| 0x7E20004C   | GPREN0     | GPIO Pin Rising Edge Detect Enable 0  | 32       | R/W |
| 0x7E200050   | GPREN1     | GPIO Pin Rising Edge Detect Enable 1  | 32       | R/W |
| 0x7E200054   | -          |               Reserved                | -        |  -  |
| 0x7E200058   | GPFEN0     | GPIO Pin Falling Edge Detect Enable 0 | 32       | R/W |
| 0x7E20005C   | GPFEN1     | GPIO Pin Falling Edge Detect Enable 1 | 32       | R/W |
| 0x7E200060   | -          |               Reserved                | -        |  -  |
| 0x7E200064   | GPHEN0     |     GPIO Pin High Detect Enable 0     | 32       | R/W |
| 0x7E200068   | GPHEN1     |     GPIO Pin High Detect Enable 1     | 32       | R/W |
| 0x7E20006C   | -          |               Reserved                | -        |  -  |
| 0x7E200070   | GPLEN0     |     GPIO Pin Low Detect Enable 0      | 32       | R/W |
| 0x7E200074   | GPLEN1     |     GPIO Pin Low Detect Enable 1      | 32       | R/W |
| 0x7E200078   | -          |               Reserved                | -        |  -  |
| 0x7E20007C   | GPAREN0    | GPIO Pin Async. Rising Edge Detect 0  | 32       | R/W |
| 0x7E200080   | GPAREN1    | GPIO Pin Async. Rising Edge Detect 1  | 32       | R/W |
| 0x7E200084   | -          |               Reserved                | -        |  -  |
| 0x7E200088   | GPAFEN0    | GPIO Pin Async. Falling Edge Detect 0 | 32       | R/W |
| 0x7E20008C   | GPAFEN1    | GPIO Pin Async. Falling Edge Detect 1 | 32       | R/W |
| 0x7E200090   | -          |               Reserved                | -        |  -  |
| 0x7E200094   | GPPUD      |     GPIO Pin Pull-up/down Enable      | 32       | R/W |
| 0x7E200098   | GPPUDCLK0  | GPIO Pin Pull-up/down Enable Clock 0  | 32       | R/W |
| 0x7E20009C   | GPPUDCLK1  | GPIO Pin Pull-up/down Enable Clock 1  | 32       | R/W |
| 0x7E2000A0   | -          |               Reserved                | -        |  -  |
| 0x7E2000B0   | -          |                 Test                  | 4        | R/W |
 ------------------------------------------------------------------------------------ */
// GPIO Registers 
#define GPFSEL0   ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200000))
#define GPFSEL1   ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200004))
#define GPFSEL2   ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200008))
#define GPFSEL3   ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x0020000C))
#define GPFSEL4   ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200010))
#define GPFSEL5   ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200014))
#define GPSET0    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x0020001C))
#define GPSET1    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200020))
#define GPCLR0    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200028))
#define GPCLR1    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x0020002C))
#define GPLEV0    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200034))
#define GPLEV1    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200038))
#define GPEDS0    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200040))
#define GPEDS1    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200044))
#define GPREN0    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x0020004C))
#define GPREN1    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200050))
#define GPFEN0    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200058))
#define GPFEN1    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x0020005C))
#define GPHEN0    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200064))
#define GPHEN1    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200068))
#define GPLEN0    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200070))
#define GPLEN1    ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200074))
#define GPAREN0   ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x0020007C))
#define GPAREN1   ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200080))
#define GPAFEN0   ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200088))
#define GPAFEN1   ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x0020008C))
#define GPPUD     ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200094))
#define GPPUDCLK0 ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x00200098))
#define GPPUDCLK1 ((unsigned int volatile *) (MMIO_BASE_ADDRESS + 0x0020009C))