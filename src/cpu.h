#ifndef CPU
#define CPU

#include <inttypes.h>

#include "memory.h"

struct cpu_s;

void cpu_reset();

void cpu_interrupt(uint16_t addr);
uint8_t cpu_execute();
void cpu_print_registers();
void cpu_print_next_instr();
uint16_t cpu_get_pc();

#endif