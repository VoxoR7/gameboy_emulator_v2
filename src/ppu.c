#include <stdlib.h>

#include "log.h"

#include "ppu.h"
#include "memory.h"
#include "fps.h"
#include "input.h"

#define INTERRUPT_IF 0xFF0F
#define INT_VBLANK  0b00'00'00'01

#define VRAM_BASE_ADDR 0x8000

#define LCDC_ADDR 0xFF40
    #define LCDC_PPU_ENABLE 0b10'00'00'00
    #define LCDC_BG_WD_TILE_DATA_AREA 0b00'01'00'00
        #define TILE_DATA_MODE_1 0x8000
        #define TILE_DATA_MODE_0 0x9000
    #define LCDC_BG_TILE_MAP_AREA 0b00'00'10'00
        #define TILE_MAP_AREA_0 0x9800
        #define TILE_MAP_AREA_1 0x9C00
    #define LCDC_OBJ_ENABLE 0b00'00'00'10
    #define LCDC_BG_WD_ENABLE 0b00'00'00'01
#define SCY_ADDR 0xFF42
#define SCX_ADDR 0xFF43
#define LY_ADDR 0xFF44

#define OAM_SCAN 2
#define OAM_SCAN_LEN (80 * 4)
#define DRAWING_PIXEL 3
#define DRAWING_PIXEL_LEN (172 * 4)
#define HORIZONTAL_BLANK 0
#define HORIZONTAL_BLANK_LEN (204 * 4)
#define VERTICAL_BLANK 1
#define VERTICAL_BLANK_LEN (4560 * 4)

struct oam_s {
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t flags;
};

uint8_t mode = OAM_SCAN;
uint8_t ly = 0;
uint8_t oam_validated = 0;
uint8_t oam_to_be_displayed[10]; // 10 is the gameboy hardware limitation

static void draw_color(uint8_t pxl_color, struct screen_s *scr, uint32_t x, uint32_t y) {
    switch (pxl_color) {
        case 0x03:
            screen_draw_pixel(scr, x, y, 0x9B, 0xBC, 0x0F);
            break;
        case 0x02:
            screen_draw_pixel(scr, x, y, 0x8B, 0xAC, 0x0F);
            break;
        case 0x01:
            screen_draw_pixel(scr, x, y, 0x30, 0x62, 0x30);
            break;
        case 0x00:
            screen_draw_pixel(scr, x, y, 0x0F, 0x38, 0x0F);
            break;
        default:
            LOG_MESG(LOG_FATAL, "Impossible color");
            exit(EXIT_FAILURE);
            break;
    }
}

static uint8_t tile_get_less_significant_bit(const uint8_t *tile, uint8_t x, uint8_t y) {
    uint8_t byte = *(tile + y * 2);
    return (byte & (1 << (7 - x))) >> (7 - x);
}

static uint8_t tile_get_most_significant_bit(const uint8_t *tile, uint8_t x, uint8_t y) {
    uint8_t byte = *(tile + y * 2 + 1);
    return ((byte & (1 << (7 - x))) >> (7 - x)) << 1;
}

static void tile_draw(struct screen_s *tiles_screen) {
    uint8_t *vram = memory_special_get_vram();

    for (uint16_t i = 0; i < 384; i++) {

        const uint16_t base_x = ((i % 16) * 8) + (i % 16);
        const uint16_t base_y = ((i / 16) * 8) + (i / 16);

        for (uint8_t j = 0; j < 8; j++)
            for (uint8_t k = 0; k < 8; k++)
                draw_color(tile_get_less_significant_bit(vram + (i * 16), k, j) + tile_get_most_significant_bit(vram + (i * 16), k, j), tiles_screen, k + base_x, j + base_y);
    }

    screen_present(tiles_screen);
}

static void map_0_draw(struct screen_s *map_screen) {
    const uint8_t bscx = memory_read_8(SCX_ADDR);
    const uint8_t bscy = memory_read_8(SCY_ADDR);

    uint8_t *vram = memory_special_get_vram();

    for (uint8_t i = 0; i < 32; i++) {
        for (uint8_t j = 0; j < 32; j++) {
            const uint32_t base_x = j * 8;
            const uint32_t base_y = i * 8;
            const uint8_t tile = *(vram + (TILE_MAP_AREA_0 - VRAM_BASE_ADDR) + j + i * 32);
            for (uint8_t k = 0; k < 8; k++)
                for (uint8_t l = 0; l < 8; l++)
                    draw_color(tile_get_less_significant_bit(vram + (tile * 16), l, k) + tile_get_most_significant_bit(vram + (tile * 16), l, k), map_screen, l + base_x, k + base_y);
        }
    }

    for (uint8_t scx = bscx, i = 0; i < 160; i += 8, scx += 8)
        screen_draw_pixel(map_screen, scx, bscy, 255, 0, 0);

    for (uint8_t scx = bscx, i = 0; i < 160; i += 8, scx += 8)
        screen_draw_pixel(map_screen, scx, (uint8_t)(bscy + 144), 255, 0, 0);

    for (uint8_t scy = bscy, i = 0; i < 144; i += 8, scy += 8)
        screen_draw_pixel(map_screen, bscx, scy, 255, 0, 0);

    for (uint8_t scy = bscy, i = 0; i < 144; i += 8, scy += 8)
        screen_draw_pixel(map_screen, (uint8_t)(bscx + 160), scy, 255, 0, 0);

    screen_present(map_screen);
}

void ppu_run(uint8_t m_cycles, struct screen_s *screen, struct screen_s *tiles_screen, struct screen_s *map_0) {
    static uint64_t m_cycles_ellapsed = 0;

    if (!(memory_read_8(LCDC_ADDR) & LCDC_PPU_ENABLE))
        return;

    m_cycles_ellapsed += m_cycles;

    switch (mode) {
        case OAM_SCAN:
            if (m_cycles_ellapsed >= OAM_SCAN_LEN) {
                // Do OAM scan
                // inside `oam_to_be_displayed`, the maximum 10 object to be displayed will be stored
                for (uint8_t i = 0; i < 40; i++) { // there is 40 tiles in the OAM area
                    struct oam_s *oam = (struct oam_s *)memory_special_get_oam_area();
                    if (oam[i].y_pos > 8 + ly && oam[i].y_pos <= 16 + ly && oam_validated < sizeof(oam_to_be_displayed)) {
                        oam_to_be_displayed[oam_validated] = i;
                        oam_validated++;
                    }
                }

                m_cycles_ellapsed -= OAM_SCAN_LEN;
                mode = DRAWING_PIXEL;
                // screen_clear(screen);
            }
            break;
        case DRAWING_PIXEL:
            if (m_cycles_ellapsed >= DRAWING_PIXEL_LEN) {

                uint8_t lcdc = memory_read_8(LCDC_ADDR);
                uint8_t *vram = memory_special_get_vram();

                /* draw background */
                if (lcdc & LCDC_BG_WD_ENABLE) {
                    const uint16_t base_map_area = lcdc & LCDC_BG_TILE_MAP_AREA ? TILE_MAP_AREA_1 - VRAM_BASE_ADDR : TILE_MAP_AREA_0 - VRAM_BASE_ADDR;
                    for (uint8_t i = 0; i < screen_get_width(screen); i++) {
                        const uint8_t x = memory_read_8(SCX_ADDR) + i;
                        const uint8_t y = memory_read_8(SCY_ADDR) + ly;

                        const uint8_t tile = *(vram + base_map_area + ((x / 8) + ((y / 8) * 32)));

                        draw_color(tile_get_less_significant_bit(vram + (tile * 16), x % 8, y % 8) + tile_get_most_significant_bit(vram + (tile * 16), x % 8, y % 8), screen, i, ly);
                    }
                }

                /* draw obj */
                if (lcdc & LCDC_OBJ_ENABLE) {
                    struct oam_s *oam = (struct oam_s *)memory_special_get_oam_area();
                    for (uint8_t i = 0; i < oam_validated; i++) {
                        // first get the line of the tile to be displayed!
                        const uint8_t y_pos = 7 - (oam[i].y_pos - (9 + ly));

                        // then get the first x pos on the screen to be displayed
                        const int16_t x_pos16 = oam[i].x_pos - 8;

                        const uint8_t tile_index = oam[i].tile_index;

                        for (int16_t x = 0; x < 8; x++) { // tiles are always 8 bits wide
                            const int16_t x_pxl = x_pos16 + x;
                            if (x_pxl < 0) // this is offscreen (on the left)
                                continue;

                            if (x_pxl >= screen_get_width(screen)) // this is offscreen (on the right)
                                continue;

                            // fetch tile color data for this pixel!
                            uint8_t *tile = vram + tile_index * 16; // one tile is 16 bytes
                            uint8_t pxl_color = (*(tile + (y_pos * 2) + 1)) & (1 << (8 - x)) ? 0x02 : 0;
                            pxl_color |= (*(tile + (y_pos * 2))) & (1 << (8 - x)) ? 0x01 : 0;

                            draw_color(pxl_color, screen, x_pos16, ly);
                        }
                    }
                }

                m_cycles_ellapsed -= DRAWING_PIXEL_LEN;
                mode = HORIZONTAL_BLANK;
            }
            break;
        case HORIZONTAL_BLANK:
            if (m_cycles_ellapsed >= HORIZONTAL_BLANK_LEN) {
                m_cycles_ellapsed -= HORIZONTAL_BLANK_LEN;
                /* make horizontal sync (or maybe only vertical sync ?)*/
                ly++;
                memory_write_8(LY_ADDR, ly);
                if (ly >= 144) {
                    // screen_present(screen);
                    memory_write_8(INTERRUPT_IF, memory_read_8(INTERRUPT_IF) | INT_VBLANK);
                    mode = VERTICAL_BLANK;
                    input_load();
                    fps_wait(16.74 * 1'000'000);
                    screen_present(screen);
                    if (tiles_screen)
                        tile_draw(tiles_screen);
                    if (map_0)
                        map_0_draw(map_0);
                } else
                    mode = OAM_SCAN;
            }
            break;
        case VERTICAL_BLANK:
            if (m_cycles_ellapsed >= OAM_SCAN_LEN + DRAWING_PIXEL_LEN + HORIZONTAL_BLANK_LEN) {
                m_cycles_ellapsed -= OAM_SCAN_LEN + DRAWING_PIXEL_LEN + HORIZONTAL_BLANK_LEN;
                ly = (ly + 1) % 154;
                memory_write_8(LY_ADDR, ly);
                if (!ly)
                    mode = OAM_SCAN;
            }
            break;
    }
}
