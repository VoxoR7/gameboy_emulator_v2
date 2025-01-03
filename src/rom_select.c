#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <dirent.h>
#include <time.h>

#include <SDL3/SDL.h>

#include "log.h"

#include "rom_select.h"
#include "input.h"

#define PATH "../rom"
#define MAX_ROM 512
#define ROM_MAX_NAME 64
#define ROM_MAX_PATH 512

char selected_rom[ROM_MAX_PATH];

char *rom_select_select(struct screen_s *screen) {
    char base_path[ROM_MAX_PATH - 16];
    strncpy(base_path, SDL_GetBasePath(), sizeof(base_path));
    base_path[sizeof(base_path) - 1] = '\0';
    strncat(base_path, PATH, sizeof(base_path) - strlen(base_path));
    base_path[sizeof(base_path) - 1] = '\0';

    uint16_t roms = 0;
    char rom_name[MAX_ROM][ROM_MAX_NAME];
    char rom_path[MAX_ROM][ROM_MAX_PATH];

    LOG_MESG(LOG_DEBUG, "Openning roms at path: %s", base_path);
    DIR *d = opendir(base_path);

    if (!d) {
        LOG_MESG(LOG_WARN, "Couldn't open rom directory: %s", base_path);
        return NULL;
    } else {
        struct dirent *dir = readdir(d);
        while (dir != NULL) {
            strncpy(rom_name[roms], dir->d_name, ROM_MAX_PATH);
            const uint32_t file_name_size = strlen(rom_name[roms]);
            const char *last = rom_name[roms] + file_name_size - 2;

            if (!strncmp(last, "gb", 2)) {
                strncpy(rom_path[roms], base_path, sizeof(rom_path[roms]));
                rom_path[roms][sizeof(rom_path[roms]) - 1] = '\0';
                size_t rom_path_len = strlen(rom_path[roms]);
                if (rom_path_len + 2 /* '/' + '\0' */ + strlen(rom_name[roms]) <= sizeof(rom_path)) {
                    rom_path[roms][rom_path_len] = '/';
                    rom_path[roms][rom_path_len + 1] = '\0';
                    strcat(rom_path[roms], rom_name[roms]);
                    roms++;
                }
            }

            dir = readdir(d);
        }

        closedir(d);
    }

    LOG_MESG(LOG_DEBUG, "Found %"PRIu16" roms", roms);

    uint16_t mid_height = screen_get_height(screen) / 2;
    uint16_t selected = 0;

    if (!roms) {
        LOG_MESG(LOG_WARN, "No roms found in rom folder!");
        screen_clear(screen);
        screen_print(screen, 0, mid_height - (FONT_HEIGHT_SIZE / 2), 255, 255, 255, " > ");
        screen_print(screen, FONT_WIDTH_SIZE * 3, mid_height - (FONT_HEIGHT_SIZE / 2), 0xD4, 0x65, 0x49, "No rom found in rom folder!");
        screen_present(screen);
        return NULL;
    }

    bool key = false;

    do {
        screen_clear(screen);
        screen_print(screen, 0, mid_height - (FONT_HEIGHT_SIZE / 2), 255, 255, 255, " > ");
        screen_print(screen, FONT_WIDTH_SIZE * 3, mid_height - (FONT_HEIGHT_SIZE / 2), 0xB3, 0xF2, 0xDD, rom_name[selected]);

        if (roms > 1) {
            screen_print(screen, 0, mid_height - (FONT_HEIGHT_SIZE / 2) - FONT_HEIGHT_SIZE - 6, 255, 255, 255, "   ");
            screen_print(screen, FONT_WIDTH_SIZE * 3, mid_height - (FONT_HEIGHT_SIZE / 2) - FONT_HEIGHT_SIZE - 6, 255, 255, 255, selected == 0 ? rom_name[roms - 1] : rom_name[selected - 1]);
        }

        if (roms > 2) {
            screen_print(screen, 0, mid_height + (FONT_HEIGHT_SIZE / 2) + 6, 255, 255, 255, "   ");
            screen_print(screen, FONT_WIDTH_SIZE * 3, mid_height + (FONT_HEIGHT_SIZE / 2) + 6, 255, 255, 255, selected == roms - 1 ? rom_name[0] : rom_name[selected + 1]);
        }

        screen_present(screen);

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 66 * 1'000'000;
        nanosleep(&ts, NULL);

        if (key) {
            key = false;
            continue;
        }
    
        input_load();
        if (input_is_pressed(INPUT_KEY_ARROW_UP)) {
            selected = selected == 0 ? roms - 1 : selected - 1;
            key = true;
        } if (input_is_pressed(INPUT_KEY_ARROW_DOWN)) {
            selected = selected == roms - 1 ? 0 : selected + 1;
            key = true;
        }
    } while (!input_is_pressed(INPUT_KEY_ENTER) && !input_is_pressed(INPUT_KEY_ESCAPE));

    if (input_is_pressed(INPUT_KEY_ESCAPE))
        return NULL;

    screen_clear(screen);
    screen_print(screen, 0, mid_height - (FONT_HEIGHT_SIZE / 2), 255, 255, 255, " > ");
    screen_print(screen, FONT_WIDTH_SIZE * 3, mid_height - (FONT_HEIGHT_SIZE / 2), 0xB3, 0xF2, 0xDD, rom_name[selected]);
    screen_present(screen);

    strncpy(selected_rom, rom_path[selected], sizeof(selected_rom));
    selected_rom[sizeof(selected_rom) - 1] = '\0';

    LOG_MESG(LOG_DEBUG, "Selected rom: '%s'", selected_rom);

    return selected_rom;
}
