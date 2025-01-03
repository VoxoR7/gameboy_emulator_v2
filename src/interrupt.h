#ifndef INTERRUPT
#define INTERRUPT

#include <inttypes.h>

void interrupt_reset();
void interrupt_disable();
void interrupt_enable();

void interrupt_run(uint8_t m_cycles);

#endif
