#include <stdlib.h>

#include "log.h"

#include "memory.h"
#include "main.h"

#define OAM_DMA_ADDR 0xFF46

#define CARTRIDGE_BANK_0 0x0000
#define CARTRIDGE_BANK_0_SIZE 0x4000
#define CARTRIDGE_BANK_N 0x4000
#define CARTRIDGE_BANK_N_SIZE 0x4000
#define VIDEO_RAM 0x8000
#define VIDEO_RAM_SIZE 0x2000
#define CARTRIDGE_RAM 0xA000
#define CARTRIDGE_RAM_SIZE 0x2000
#define WORK_RAM_0 0xC000
#define WORK_RAM_0_SIZE 0x1000
#define WORK_RAM_N 0xD000
#define WORK_RAM_N_SIZE 0x1000
#define ECHO_RAM 0xE000
#define ECHO_RAM_SIZE 0x0E00
#define OAM_RAM 0xFE00
#define OAM_RAM_SIZE 0x00A0
#define UNUSABLE 0xFEA0
#define UNUSABLE_SIZE 0x0060
#define IO 0xFF00
#define IO_SIZE 0x0080
#define HIGH_RAM 0xFF80
#define HIGH_RAM_SIZE 0x007F
#define INTERRUPT_ENABLE 0xFFFF
#define INTERRUPT_ENABLE_SIZE 0x0001

struct memory_s {
    uint8_t *cartridge_bank_0;
    uint8_t *cartridge_bank_n;
    uint8_t video_ram[VIDEO_RAM_SIZE];
    // uint8_t cartridge_ram;
    uint8_t work_ram_0[WORK_RAM_0_SIZE];
    uint8_t work_ram_n[WORK_RAM_N_SIZE];
    uint8_t oam_ram[OAM_RAM_SIZE];
    uint8_t io[IO_SIZE];
    uint8_t high_ram[HIGH_RAM_SIZE];
    uint8_t intterupt_enable;
};

struct memory_s memory;

void memory_reset() {
    memory.cartridge_bank_0 = NULL;
    memory.cartridge_bank_n = NULL;
}

void memory_cartridge_load(struct cartridge_s *cartridge) {
    memory.cartridge_bank_0 = cartridge_get_bank(cartridge, 0);
    memory.cartridge_bank_n = cartridge_get_bank(cartridge, 1);

    if (!memory.cartridge_bank_0 || !memory.cartridge_bank_n) {
        LOG_MESG(LOG_WARN, "Couldn't load cartridge banks into memory");
    }
}

uint8_t memory_read_8(uint16_t addr) {
    if (addr < CARTRIDGE_BANK_0 + CARTRIDGE_BANK_0_SIZE)
        return *(memory.cartridge_bank_0 + addr);
    if (addr >= CARTRIDGE_BANK_N && addr < CARTRIDGE_BANK_N + CARTRIDGE_BANK_N_SIZE)
        return *(memory.cartridge_bank_n + (addr - CARTRIDGE_BANK_N));
    if (addr >= VIDEO_RAM && addr < VIDEO_RAM + VIDEO_RAM_SIZE)
        return memory.video_ram[addr - VIDEO_RAM];
    if (addr >= WORK_RAM_0 && addr < WORK_RAM_0 + WORK_RAM_0_SIZE)
        return memory.work_ram_0[addr - WORK_RAM_0];
    if (addr >= WORK_RAM_N && addr < WORK_RAM_N + WORK_RAM_N_SIZE)
        return memory.work_ram_n[addr - WORK_RAM_N];
    if (addr >= OAM_RAM && addr < OAM_RAM + OAM_RAM_SIZE)
        return memory.oam_ram[addr - OAM_RAM];
    if (addr >= IO && addr < IO + IO_SIZE)
        return memory.io[addr - IO];
    if (addr >= HIGH_RAM && addr < HIGH_RAM + HIGH_RAM_SIZE)
        return memory.high_ram[addr - HIGH_RAM];
    if (addr == INTERRUPT_ENABLE)
        return memory.intterupt_enable;

    LOG_MESG(LOG_FATAL, "Couldn't read at addr 0x%04X", addr);
    exit(EXIT_FAILURE);
}

static void start_oam_dma(uint8_t src) {
    for (uint8_t i = 0; i < 0xA0; i++)
        memory.oam_ram[i] = memory_read_8((((uint16_t)src) << 8) + i);

    main_add_m_cycles(160);
}

void memory_write_8(uint16_t addr, uint8_t value) {
    if (addr < CARTRIDGE_BANK_0 + CARTRIDGE_BANK_0_SIZE)
        *(memory.cartridge_bank_0 + addr) = value;
    else if (addr < CARTRIDGE_BANK_N + CARTRIDGE_BANK_N_SIZE)
        *(memory.cartridge_bank_n + (addr - CARTRIDGE_BANK_N)) = value;
    else if (addr >= VIDEO_RAM && addr < VIDEO_RAM + VIDEO_RAM_SIZE)
        memory.video_ram[addr - VIDEO_RAM] = value;
    else if (addr >= WORK_RAM_0 && addr < WORK_RAM_0 + WORK_RAM_0_SIZE)
        memory.work_ram_0[addr - WORK_RAM_0] = value;
    else if (addr >= WORK_RAM_N && addr < WORK_RAM_N + WORK_RAM_N_SIZE)
        memory.work_ram_n[addr - WORK_RAM_N] = value;
    else if (addr >= OAM_RAM && addr < OAM_RAM + OAM_RAM_SIZE)
        memory.oam_ram[addr - OAM_RAM] = value;
    else if (addr >= UNUSABLE && addr < UNUSABLE + UNUSABLE_SIZE)
        LOG_MESG(LOG_WARN, "Writing in a forbidden area! (0x%04X)", addr);
    else if (addr == OAM_DMA_ADDR)
        start_oam_dma(value);
    else if (addr >= IO && addr < IO + IO_SIZE)
        memory.io[addr - IO] = value;
    else if (addr >= HIGH_RAM && addr < HIGH_RAM + HIGH_RAM_SIZE)
        memory.high_ram[addr - HIGH_RAM] = value;
    else if (addr == INTERRUPT_ENABLE)
        memory.intterupt_enable = value;
    else {
        LOG_MESG(LOG_FATAL, "Couldn't write at addr 0x%04X", addr);
        exit(EXIT_FAILURE);
    }
}

uint16_t memory_read_16(uint16_t addr) {
    if (addr < CARTRIDGE_BANK_0 + CARTRIDGE_BANK_0_SIZE - 1)
        return (uint16_t)(*(memory.cartridge_bank_0 + addr) + (*(memory.cartridge_bank_0 + addr + 1) << 8));
    if (addr < CARTRIDGE_BANK_N + CARTRIDGE_BANK_N_SIZE - 1)
        return (uint16_t)(*(memory.cartridge_bank_n + (addr - CARTRIDGE_BANK_N)) + (*(memory.cartridge_bank_n + (addr - CARTRIDGE_BANK_N) + 1) << 8));
    else if (addr >= WORK_RAM_0 && addr < WORK_RAM_0 + WORK_RAM_0_SIZE - 1)
        return (uint16_t)(*(memory.work_ram_0 + (addr - WORK_RAM_0)) + (*(memory.work_ram_0 + (addr - WORK_RAM_0) + 1) << 8));

    LOG_MESG(LOG_FATAL, "Couldn't read at addr 0x%04X", addr);
    exit(EXIT_FAILURE);
}

void memory_write_16(uint16_t addr, uint16_t value) {
    if (addr >= WORK_RAM_0 && addr < WORK_RAM_0 + WORK_RAM_0_SIZE - 1) { // 4KB Work RAM Bank 0 (WRAM)
        memory.work_ram_0[addr - WORK_RAM_0 + 1] = (uint8_t)(value >> 8);
        memory.work_ram_0[addr - WORK_RAM_0] = (uint8_t)(value & 0xFF);
        return;
    }

    LOG_MESG(LOG_FATAL, "Couldn't write at addr 0x%04X", addr);
    exit(EXIT_FAILURE);
}

uint8_t *memory_special_get_oam_area() {
    return memory.oam_ram;
}

uint8_t *memory_special_get_vram() {
    return memory.video_ram;
}