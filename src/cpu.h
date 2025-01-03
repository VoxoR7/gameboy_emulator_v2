#ifndef CPU
#define CPU

#include <inttypes.h>

#include "memory.h"

struct cpu_s;

void cpu_reset();

void cpu_interrupt(uint16_t addr);
uint8_t cpu_execute();

#endif