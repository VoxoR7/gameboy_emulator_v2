#include <inttypes.h>

#include "log.h"

#include "timer.h"
#include "memory.h"

#define TIMER_DIV_MEMORY_ADDR 0xFF04
#define TIMER_DIV_HERTZ_CLOCK 16'384
#define TIMER_DIV_MACHINE_CLOCK (TIMER_DIV_HERTZ_CLOCK / 4)

static void timer_div(uint8_t m_cycles) {
    static uint64_t m_cycles_ellapsed = 0;
    m_cycles_ellapsed += m_cycles;

    if (m_cycles_ellapsed >= TIMER_DIV_MACHINE_CLOCK) {
        m_cycles_ellapsed -= TIMER_DIV_MACHINE_CLOCK;
        memory_write_8(TIMER_DIV_MEMORY_ADDR, memory_read_8(TIMER_DIV_MEMORY_ADDR) + 1);
    }
}

#define TIMER_TIMA_MEMORY_ADDR 0xFF05
#define TIMER_TMA_MEMORY_ADDR 0xFF06
#define TIMER_TAC_MEMORY_ADDR 0xFF07

#define TIMER_TAC_CLOCK_SELECT 0b00'00'00'11
#define TIMER_TAC_TIMA_ENABLE 0b00'00'01'00

#define TIMER_TIMA_00_MACHINE_CLOCK 256
#define TIMER_TIMA_01_MACHINE_CLOCK 4
#define TIMER_TIMA_10_MACHINE_CLOCK 16
#define TIMER_TIMA_11_MACHINE_CLOCK 64

static bool timer_inc_tima() {
    uint8_t tima = memory_read_8(TIMER_TIMA_MEMORY_ADDR);
    if (tima == 0xFF) {
        memory_write_8(TIMER_TIMA_MEMORY_ADDR, memory_read_8(TIMER_TMA_MEMORY_ADDR));
        return true;
    }

    memory_write_8(TIMER_TIMA_MEMORY_ADDR, tima + 1);
    return false;
}

static bool timer_tima(uint8_t m_cycles) {
    static uint64_t m_cycles_ellapsed = 0;

    if (!(memory_read_8(TIMER_TAC_MEMORY_ADDR) & TIMER_TAC_TIMA_ENABLE))
        return false;

    m_cycles_ellapsed += m_cycles;

    uint8_t clock_select = memory_read_8(TIMER_TAC_MEMORY_ADDR) & TIMER_TAC_CLOCK_SELECT;
    switch (clock_select) {
        case 0x00:
            if (m_cycles_ellapsed >= TIMER_TIMA_00_MACHINE_CLOCK) {
                m_cycles_ellapsed -= TIMER_TIMA_00_MACHINE_CLOCK;
                return timer_inc_tima();
            }
            return false;
        case 0b01:
            if (m_cycles_ellapsed >= TIMER_TIMA_01_MACHINE_CLOCK) {
                m_cycles_ellapsed -= TIMER_TIMA_01_MACHINE_CLOCK;
                return timer_inc_tima();
            }
            return false;
        case 0b10:
            if (m_cycles_ellapsed >= TIMER_TIMA_10_MACHINE_CLOCK) {
                m_cycles_ellapsed -= TIMER_TIMA_10_MACHINE_CLOCK;
                return timer_inc_tima();
            }
            return false;
        case 0b11:
            if (m_cycles_ellapsed >= TIMER_TIMA_11_MACHINE_CLOCK) {
                m_cycles_ellapsed -= TIMER_TIMA_11_MACHINE_CLOCK;
                return timer_inc_tima();
            }
            return false;
    }

    return false;
}

bool timer_run(uint8_t m_cycles) {
    timer_div(m_cycles);
    return timer_tima(m_cycles);
}