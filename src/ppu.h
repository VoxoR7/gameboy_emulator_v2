#ifndef PPU
#define PPU

#include <inttypes.h>

#include "screen.h"

void ppu_run(uint8_t m_cycles, struct screen_s *screen, struct screen_s *tiles_screen, struct screen_s *map_0);

#endif