#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "log.h"

#include "screen.h"

#define FONT_SIZE 24

struct screen_s {
    uint16_t width;
    uint16_t height;
    uint16_t scale;
    SDL_Window *window;
    SDL_Renderer *renderer;
};

TTF_Font *font = NULL;

bool screen_global_init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_MESG(LOG_WARN, "Couldn't initialize SDL: %s", SDL_GetError());
        return false;
    }

    if(!TTF_Init()) {
        LOG_MESG(LOG_WARN, "Couldn't initialize TTF");
        SDL_Quit();
        return false;
    }

    char font_path[512];
    strncpy(font_path, SDL_GetBasePath(), sizeof(font_path));
    font_path[sizeof(font_path) - 1] = '\0';
    strncat(font_path, "../data/Nexa-Heavy.ttf", sizeof(font_path) - strlen(font_path));
    font_path[sizeof(font_path) - 1] = '\0';

    LOG_MESG(LOG_DEBUG, "Openning font at path: %s", font_path);

    font = TTF_OpenFont(font_path, FONT_SIZE);
    if (!font) {
        LOG_MESG(LOG_WARN, "Couldn't open font");
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}

void screen_global_shutdown() {
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}

struct screen_s *screen_create(char *title, uint16_t width, uint16_t height, uint16_t scale) {
    struct screen_s *screen = malloc(sizeof(struct screen_s));
    if (!screen)
        return NULL;

    screen->scale = scale;
    screen->width = width;
    screen->height = height;

    if (!SDL_CreateWindowAndRenderer(title, width * scale, height * scale, 0, &screen->window, &screen->renderer)) {
        LOG_MESG(LOG_WARN, "Couldn't create window and renderer: %s", SDL_GetError());
        free(screen);
        return NULL;
    }

    return screen;

    return NULL;
}

void screen_clear(struct screen_s *screen) {
    SDL_SetRenderDrawColor(screen->renderer, 0, 0, 0, 0);
    SDL_RenderClear(screen->renderer);
}

uint16_t screen_get_width(struct screen_s *screen) {
    return screen->width;
}

uint16_t screen_get_height(struct screen_s *screen) {
    return screen->height;
}

void screen_print(struct screen_s *screen, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, char *msg) {
    SDL_Color color = {r, g, b, 255};
    SDL_Color bgcolor = {0, 0, 0, 255};

    size_t msg_len = strlen(msg);
    SDL_Surface *text_surface = TTF_RenderText_Shaded(font, msg, msg_len, color, bgcolor);
    if(!text_surface)
        LOG_MESG(LOG_WARN, "Couldn't create surface from text");
    else {
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(screen->renderer, text_surface);
        SDL_DestroySurface(text_surface);
        if (!text_texture) {
            LOG_MESG(LOG_WARN, "Couldn't create texture from surface: %s", SDL_GetError());
            return;
        }

        SDL_FRect dst_rect;
        dst_rect.x = x * screen->scale;
        dst_rect.y = y * screen->scale;
        dst_rect.h = screen->scale * FONT_HEIGHT_SIZE;
        dst_rect.w = msg_len * screen->scale * FONT_WIDTH_SIZE;

        if (!SDL_RenderTexture(screen->renderer, text_texture, NULL, &dst_rect))
            LOG_MESG(LOG_WARN, "Couldn't blit surface: %s", SDL_GetError());

        SDL_DestroyTexture(text_texture);
    }
}

void screen_draw_pixel(struct screen_s *screen, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b) {
    SDL_SetRenderDrawColor(screen->renderer, r, g, b, 255);
    SDL_FRect rect = {
        .x = x * screen->scale,
        .y = y * screen->scale,
        .w = screen->scale,
        .h = screen->scale
    };

    SDL_RenderFillRect(screen->renderer, &rect);

    /*for (uint32_t i = 0; i < screen->scale; i++)
        for (uint32_t j = 0; j < screen->scale; j++)
            SDL_RenderPoint(screen->renderer, x * screen->scale + i, y * screen->scale + j);*/
}

void screen_present(struct screen_s *screen) {
    SDL_RenderPresent(screen->renderer);
    
}

void screen_destroy(struct screen_s *screen) {
    SDL_DestroyRenderer(screen->renderer);
    SDL_DestroyWindow(screen->window);
    free(screen);
}
