#ifndef _DEF_SIGNAL
#define _DEF_SIGNAL
#include <stdint.h>

#define SIGNAL_NUM 30

struct SignalHandler_{
    uint8_t signum;
    void (*handler)();
    struct SignalHandler_ *next;
};
typedef struct SignalHandler_ SignalHandler;
void signal_handler(uint8_t SIGNAL, void (*handler)());
void signal_kill(uint64_t pid, uint8_t SIGNAL);
void signal_sigreturn();
void signal_handler_exec(uint8_t SIGNAL);
void signal_check();

#endif