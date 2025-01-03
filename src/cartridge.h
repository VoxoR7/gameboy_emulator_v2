#ifndef CARTRIDGE
#define CARTRIDGE

#include <inttypes.h>

#define BANK_SIZE 0x4000

struct cartridge_s;

struct cartridge_s *cartridge_load(char *path);
void cartridge_unload(struct cartridge_s *cartridge);

uint8_t *cartridge_get_bank(struct cartridge_s *cartridge, uint8_t bank);

#endif
