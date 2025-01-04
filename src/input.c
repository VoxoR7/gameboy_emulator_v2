#include <SDL3/SDL.h>

#include "input.h"

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
            case SDLK_Z:
                status[INPUT_KEY_Z] = pressed;
                break;
            case SDLK_S:
                status[INPUT_KEY_S] = pressed;
                break;
        }
    }
}

bool input_is_pressed(enum input_key_e query) {
    return status[query];
}
