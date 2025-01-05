#include <stdlib.h>

#include <SDL3/SDL.h>

#include "log.h"

#include "input.h"
#include "memory.h"

bool status[INPUT_KEY_END];

void input_load() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        bool pressed;
        if (e.type == SDL_EVENT_KEY_DOWN)
            pressed = true;
        else if (e.type == SDL_EVENT_KEY_UP)
            pressed = false;
        else
            continue;
            
        switch (e.key.key) {
            case SDLK_ESCAPE:
                status[INPUT_KEY_ESCAPE] = pressed;
                break;
            case SDLK_UP:
                status[INPUT_KEY_ARROW_UP] = pressed;
                break;
            case SDLK_DOWN:
                status[INPUT_KEY_ARROW_DOWN] = pressed;
                break;
            case SDLK_RETURN:
                status[INPUT_KEY_ENTER] = pressed;
                break;
            case SDLK_BACKSPACE:
                status[INPUT_KEY_BACKSPACE] = pressed;
                break;
            case SDLK_Z:
                status[INPUT_KEY_Z] = pressed;
                break;
            case SDLK_S:
                status[INPUT_KEY_S] = pressed;
                break;
            case SDLK_Q:
                status[INPUT_KEY_Q] = pressed;
                break;
            case SDLK_D:
                status[INPUT_KEY_D] = pressed;
                break;
            case SDLK_P:
                status[INPUT_KEY_P] = pressed;
                break;
            case SDLK_L:
                status[INPUT_KEY_L] = pressed;
                break;
        }
    }
}

bool input_is_pressed(enum input_key_e query) {
    return status[query];
}

#define JOYPAD_ADDR 0xFF00
#define SELECT_D_PAD 0x10
    #define RIGHT 0x01
    #define LEFT 0x02
    #define UP 0x04
    #define DOWN 0x08
#define SELECT_BUTTONS 0x20
    #define A 0x01
    #define B 0x02
    #define SELECT 0x04
    #define START 0x08

void input_run() {
    static bool startup = true;
    uint8_t select = memory_read_8(JOYPAD_ADDR);
    if ((!(select & SELECT_D_PAD)) && (!(select & SELECT_BUTTONS))) {
        if (startup)
            return;
        LOG_MESG(LOG_WARN, "Both d-pad and buttons are selected");
        exit(EXIT_FAILURE);
    }

    startup = false;

    select |= 0xCF;

    if (!(select & SELECT_D_PAD)) {
        if (status[INPUT_KEY_D])
            select &= ~RIGHT;
        if (status[INPUT_KEY_Q])
            select &= ~LEFT;
        if (status[INPUT_KEY_Z])
            select &= ~UP;
        if (status[INPUT_KEY_S])
            select &= ~DOWN;
    } else if (!(select & SELECT_BUTTONS)) {
        if (status[INPUT_KEY_P])
            select &= ~A;
        if (status[INPUT_KEY_L])
            select &= ~B;
        if (status[INPUT_KEY_BACKSPACE])
            select &= ~SELECT;
        if (status[INPUT_KEY_ENTER])
            select &= ~START;
    }

    memory_write_8(JOYPAD_ADDR, select);
}
