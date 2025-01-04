#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "log.h"

#include "screen.h"
#include "rom_select.h"
#include "input.h"
#include "cartridge.h"
#include "memory.h"
#include "cpu.h"
#include "interrupt.h"
#include "cpu_debug.h"

int main() {
    log_init(LOG_DEBUG, NULL);

    LOG_MESG(LOG_INFO, "VoxoR Gameboy emulator");

    screen_global_init();

    struct screen_s *gb_screen = screen_create("VoxoR Gameboy Emulator", 160, 144, 4);
    if (!gb_screen) {
        LOG_MESG(LOG_FATAL, "Couldn't create gameboy screen");
        screen_global_shutdown();
        exit(EXIT_FAILURE);
    }

    char *rom = rom_select_select(gb_screen);
    if (!rom) {
        if (input_is_pressed(INPUT_KEY_ESCAPE)) {
            screen_destroy(gb_screen);
            screen_global_shutdown();
            exit(EXIT_SUCCESS);
        }

        while (!input_is_pressed(INPUT_KEY_ESCAPE)) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 66 * 1'000'000;
            nanosleep(&ts, NULL);
            input_load();
        }

        screen_destroy(gb_screen);
        screen_global_shutdown();
        exit(EXIT_FAILURE);
    }

    struct cartridge_s *cartridge = cartridge_load(rom);
    if (cartridge == NULL) {
        LOG_MESG(LOG_FATAL, "Couldn't load cartridge");
        screen_destroy(gb_screen);
        screen_global_shutdown();
        exit(EXIT_FAILURE);
    }

    memory_reset();
    memory_cartridge_load(cartridge);
    cpu_reset();
    interrupt_reset();

    uint64_t m_cycles_total = 0, instruction_executed = 0;

    do {
        if (!cpu_debug_run())
            break;
        uint8_t m_cycles = cpu_execute();
        if (!m_cycles) {
            LOG_MESG(LOG_FATAL, "cpu failed to execute!");
            LOG_MESG(LOG_FATAL, "m cycles elapsed: %"PRIu64", instructions executed: %"PRIu64"", m_cycles_total, instruction_executed);
            exit(EXIT_FAILURE);
        }
        m_cycles_total += m_cycles;
        instruction_executed++;
        interrupt_run(m_cycles);
        input_load();
    } while(!input_is_pressed(INPUT_KEY_ESCAPE));

    LOG_MESG(LOG_INFO, "m cycles elapsed: %"PRIu64", instructions executed: %"PRIu64"", m_cycles_total, instruction_executed);

    cartridge_unload(cartridge);
    screen_destroy(gb_screen);
    screen_global_shutdown();
    exit(EXIT_SUCCESS);
}
