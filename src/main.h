#ifndef MAIN
#define MAIN

#define GAMEBOY_HERTZ_CLOCK 4'194'304
#define GAMEBOY_MACHINE_CLOCK (GAMEBOY_HERTZ_CLOCK / 4)

void main_add_m_cycles(uint8_t m_cycles);

#endif