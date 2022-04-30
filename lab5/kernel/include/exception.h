#pragma once

#include "gpio.h"
#include "utils.h"
#include <stdint.h>
void sync_handler_currentEL_ELx();
void sync_handler_lowerEL_64(uint64_t sp_addr);
void irq_handler_currentEL_ELx();
void irq_handler_lowerEL_64();
void default_handler();
void enable_interrupt();
void disable_interrupt();