#pragma once

#include "gpio.h"

void sync_handler();
void el1_to_el1_irq_handler();
void el0_to_el1_irq_handler();
void default_handler();
void enable_interrupt();
void disable_interrupt();