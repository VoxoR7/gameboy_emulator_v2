#ifndef SCREEN
#define SCREEN

#include <inttypes.h>

#define FONT_WIDTH_SIZE 4
#define FONT_HEIGHT_SIZE 8

struct screen_s;

bool screen_global_init();
void screen_global_shutdown();

struct screen_s *screen_create(char *title, uint16_t width, uint16_t height, uint16_t scale);
void screen_destroy(struct screen_s *screen);

uint16_t screen_get_width(struct screen_s *screen);
uint16_t screen_get_height(struct screen_s *screen);
void screen_clear(struct screen_s *screen);
void screen_draw_pixel(struct screen_s *screen, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);
void screen_print(struct screen_s *screen, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, char *msg);
void screen_present(struct screen_s *screen);

#endif