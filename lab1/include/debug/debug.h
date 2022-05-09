#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "utils.h"

#define DEBUG_KERNEL_START() log_kernel_start()
#define LOG(fmt, ...) \
    if(debug) \
        printf("[%s/%s:%d] " fmt "\r\n" , __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    


extern void log_kernel_start(void);

extern int debug;
#endif
