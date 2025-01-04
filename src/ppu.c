#include <stdlib.h>

#include "log.h"

#include "ppu.h"
#include "memory.h"

#define LCDC_ADDR 0xFF40
    #define LCDC_PPU_ENABLE 0b10'00'00'00
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

void ppu_run(uint8_t m_cycles, struct screen_s *screen) {
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
            }
            break;
        case DRAWING_PIXEL:
            if (m_cycles_ellapsed >= DRAWING_PIXEL_LEN) {
                struct oam_s *oam = (struct oam_s *)memory_special_get_oam_area();
                uint8_t *vram = memory_special_get_vram();
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

                        switch (pxl_color) {
                            case 0x03:
                                screen_draw_pixel(screen, x_pos16, ly, 0x9B, 0xBC, 0x0F);
                                break;
                            case 0x02:
                                screen_draw_pixel(screen, x_pos16, ly, 0x8B, 0xAC, 0x0F);
                                break;
                            case 0x01:
                                screen_draw_pixel(screen, x_pos16, ly, 0x30, 0x62, 0x30);
                                break;
                            case 0x00:
                                screen_draw_pixel(screen, x_pos16, ly, 0x0F, 0x38, 0x0F);
                                break;
                            default:
                                LOG_MESG(LOG_FATAL, "Impossible color");
                                exit(EXIT_FAILURE);
                                break;
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
                if (ly >= 144)
                    mode = VERTICAL_BLANK;
                else
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
