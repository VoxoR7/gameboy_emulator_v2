#ifndef MEMORY
#define MEMORY

#include <inttypes.h>

#include "cartridge.h"

struct memory_s;

void memory_reset();

void memory_cartridge_load(struct cartridge_s *cartridge);
uint8_t memory_read_8(uint16_t addr);
void memory_write_8(uint16_t addr, uint8_t value);
uint16_t memory_read_16(uint16_t addr);
void memory_write_16(uint16_t addr, uint16_t value);

#endif