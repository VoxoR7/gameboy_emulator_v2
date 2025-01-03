#include "log.h"

#include "interrupt.h"
#include "timer.h"
#include "memory.h"
#include "cpu.h"

#define INTERRUPT_IF 0xFF0F
#define INTERRUPT_IE 0xFFFF

#define ALL_INTERRUPT 0b00'01'11'11

#define INT_VBLANK  0b00'00'00'01
#define INT_LCD     0b00'00'00'10
#define INT_TIMER   0b00'00'01'00
#define INT_SERIAL  0b00'00'10'00
#define INT_JOYPAD  0b00'01'00'00

#define INTERRUPT_ADDR_VBLANK   0x40
#define INTERRUPT_ADDR_LCDSTAT  0x48
#define INTERRUPT_ADDR_TIMER    0x50
#define INTERRUPT_ADDR_SERIAL   0x58
#define INTERRUPT_ADDR_JOYPAD   0x60

bool ime;

void interrupt_reset() {
    ime = true;
}

void interrupt_disable() {
    ime = false;
}

void interrupt_enable() {
    ime = true;
}

void interrupt_run(uint8_t m_cycles) {
    if (timer_run(m_cycles))
        memory_write_8(INTERRUPT_IF, memory_read_8(INTERRUPT_IF) | INT_TIMER);

    if (!ime)
        return;

    uint8_t interrupt_ready = memory_read_8(INTERRUPT_IF) & memory_read_8(INTERRUPT_IE) & ALL_INTERRUPT;

    switch (interrupt_ready) {
        case INT_VBLANK:
            return;
        case INT_LCD:
            return;
        case INT_TIMER:
            memory_write_8(INTERRUPT_IF, memory_read_8(INTERRUPT_IF) & ~INT_TIMER);
            interrupt_disable();
            cpu_interrupt(INTERRUPT_ADDR_TIMER);
            return;
        case INT_SERIAL:
            return;
        case INT_JOYPAD:
            return;
        default:
            break;
    }
}
