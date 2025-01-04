#include <stdlib.h>
#include <stdio.h>

#include "log.h"

#include "cpu.h"
#include "interrupt.h"

#define FLAGS_Z 0b1000'0000
#define FLAGS_N 0b0100'0000
#define FLAGS_H 0b0010'0000
#define FLAGS_C 0b0001'0000

#define FLAGS_RST 0b0000'0000
#define FLAGS_ALL 0b1111'0000

struct registers_s {
    struct {
        union {
            struct {
                uint8_t f;
                uint8_t a;
            };
            uint16_t af;
        };
    };

    struct {
        union {
            struct {
                uint8_t c;
                uint8_t b;
            };
            uint16_t bc;
        };
    };

    struct {
        union {
            struct {
                uint8_t e;
                uint8_t d;
            };
            uint16_t de;
        };
    };

    struct {
        union {
            struct {
                uint8_t l;
                uint8_t h;
            };
            uint16_t hl;
        };
    };

    uint16_t sp;
    uint16_t pc;
};

struct cpu_s {
    struct registers_s registers;
};

struct cpu_s cpu;

void cpu_reset() {
    cpu.registers.pc = 0x100;
}

void cpu_interrupt(uint16_t addr) {
    cpu.registers.sp -= 2;
    memory_write_16(cpu.registers.sp, cpu.registers.pc);
    cpu.registers.pc = addr;
}

void cpu_print_registers() {
    printf("\taf: 0x%04X\n", cpu.registers.af);
    printf("\tbc: 0x%04X\n", cpu.registers.bc);
    printf("\tde: 0x%04X\n", cpu.registers.de);
    printf("\thl: 0x%04X\n", cpu.registers.hl);
    printf("\tsp: 0x%04X\n", cpu.registers.sp);
    printf("\tpc: 0x%04X\n", cpu.registers.pc);
}

void cpu_print_next_instr() {
    uint8_t instr = memory_read_8(cpu.registers.pc);
    char msg[512];
    sprintf(msg, "next instruction at pc 0x%04X, instruction 0x%02X -> ", cpu.registers.pc, instr);
    switch (instr) {
        case 0x00:
            LOG_MESG(LOG_DEBUG, "%s NOP", msg);
            break;
        case 0x01:
            LOG_MESG(LOG_DEBUG, "%s LD BC, d16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0x05:
            LOG_MESG(LOG_DEBUG, "%s DEC B", msg);
            break;
        case 0x06:
            LOG_MESG(LOG_DEBUG, "%s LD B, d8 (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0x0B:
            LOG_MESG(LOG_DEBUG, "%s DEC BC", msg);
            break;
        case 0x0C:
            LOG_MESG(LOG_DEBUG, "%s INC C", msg);
            break;
        case 0x0D:
            LOG_MESG(LOG_DEBUG, "%s DEC C", msg);
            break;
        case 0x0E:
            LOG_MESG(LOG_DEBUG, "%s LD C, d8 (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0x20:
            LOG_MESG(LOG_DEBUG, "%s JR NZ, r8 (%d)", msg, (int8_t)memory_read_8(cpu.registers.pc + 1));
            break;
        case 0x21:
            LOG_MESG(LOG_DEBUG, "%s LD HL, d16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0x2A:
            LOG_MESG(LOG_DEBUG, "%s LD A, (HL); INC HL", msg);
            break;
        case 0x31:
            LOG_MESG(LOG_DEBUG, "%s LD SP, d16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0x32:
            LOG_MESG(LOG_DEBUG, "%s LD (HL), A; DEC HL", msg);
            break;
        case 0x36:
            LOG_MESG(LOG_DEBUG, "%s LD (HL), d8 (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0x3E:
            LOG_MESG(LOG_DEBUG, "%s LD A, d8 (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0x78:
            LOG_MESG(LOG_DEBUG, "%s LD A, B", msg);
            break;
        case 0xAF:
            LOG_MESG(LOG_DEBUG, "%s XOR A, A", msg);
            break;
        case 0xB1:
            LOG_MESG(LOG_DEBUG, "%s OR C", msg);
            break;
        case 0xC3:
            LOG_MESG(LOG_DEBUG, "%s JP a16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0xC9:
            LOG_MESG(LOG_DEBUG, "%s RET", msg);
            break;
        case 0xCD:
            LOG_MESG(LOG_DEBUG, "%s CALL a16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0xE0:
            LOG_MESG(LOG_DEBUG, "%s LD ($FF00+a8), A (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0xE2:
            LOG_MESG(LOG_DEBUG, "%s ($FF00+C), A", msg);
            break;
        case 0xEA:
            LOG_MESG(LOG_DEBUG, "%s LD (a16), A (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0xF0:
            LOG_MESG(LOG_DEBUG, "%s LD A, ($FF00+a8) (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0xF3:
            LOG_MESG(LOG_DEBUG, "%s DI", msg);
            break;
        case 0xFE:
            LOG_MESG(LOG_DEBUG, "%s CP d8 (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        default:
            LOG_MESG(LOG_DEBUG, "%s Unknow next instruction", msg, instr);
            break;
    }
}

uint16_t cpu_get_pc() {
    return cpu.registers.pc;
}

uint8_t cpu_execute() {
    uint8_t instr = memory_read_8(cpu.registers.pc);
    // LOG_MESG(LOG_DEBUG, "Instruction 0x%02X (d8 0x%02X, d16 0x%04X) at pc 0x%04X", instr, memory_read_8(cpu.registers.pc), memory_read_16(cpu.registers.pc), cpu.registers.pc);
    cpu.registers.pc++;

    uint8_t d8;

    switch (instr) {
        case 0x00: /* NOP */
            return 1;
        case 0x01: /* LD BC, d16 */
            cpu.registers.bc = memory_read_16(cpu.registers.pc);
            cpu.registers.pc += 2;
            return 3;
        case 0x05: /* DEC B */
            cpu.registers.f |= FLAGS_N;
            if (((cpu.registers.b & 0x0F) - 1) & 0xF0)
                cpu.registers.f |= FLAGS_H;
            else
                cpu.registers.f &= ~FLAGS_H;
            cpu.registers.b--;
            if (!cpu.registers.b)
                cpu.registers.f |= FLAGS_Z;
            else
                cpu.registers.f &= ~FLAGS_Z;
            return 1;
        case 0x06: /* LD B, d8 */
            cpu.registers.b = memory_read_8(cpu.registers.pc);
            cpu.registers.pc += 1;
            return 2;
        case 0x0B: /* DEC BC */
            cpu.registers.bc--;
            return 2;
        case 0x0C: /* INC C */
            cpu.registers.f &= ~FLAGS_N;
            if (((cpu.registers.c & 0x0F) + 1) & 0xF0)
                cpu.registers.f |= FLAGS_H;
            else
                cpu.registers.f &= ~FLAGS_H;
            cpu.registers.c++;
            if (!cpu.registers.c)
                cpu.registers.f |= FLAGS_Z;
            else
                cpu.registers.f &= ~FLAGS_Z;
            return 1;
        case 0x0D: /* DEC C */
            cpu.registers.f |= FLAGS_N;
            if (((cpu.registers.c & 0x0F) - 1) & 0xF0)
                cpu.registers.f |= FLAGS_H;
            else
                cpu.registers.f &= ~FLAGS_H;
            cpu.registers.c--;
            if (!cpu.registers.c)
                cpu.registers.f |= FLAGS_Z;
            else
                cpu.registers.f &= ~FLAGS_Z;
            return 1;
        case 0x0E: /* LD C, d8 */
            cpu.registers.c = memory_read_8(cpu.registers.pc);
            cpu.registers.pc += 1;
            return 2;
        case 0x20: /* JR NZ, r8 */
            int8_t r8 = (int8_t)memory_read_8(cpu.registers.pc);
            cpu.registers.pc++;
            if (cpu.registers.f & FLAGS_Z)
                return 2;

            cpu.registers.pc += r8;
            return 3;
        case 0x21: /* LD HL, d16 */
            cpu.registers.hl = memory_read_16(cpu.registers.pc);
            cpu.registers.pc += 2;
            return 3;
        case 0x2A: /* LD A, (HL); INC HL */
            cpu.registers.a = memory_read_8(cpu.registers.hl);
            cpu.registers.hl++;
            return 2;
        case 0x31: /* LD SP, d16 */
            cpu.registers.sp = memory_read_16(cpu.registers.pc);
            cpu.registers.pc += 2;
            return 3;
        case 0x32: /* LD (HL), A; DEC HL */
            memory_write_8(cpu.registers.hl, cpu.registers.a);
            cpu.registers.hl--;
            return 2;
        case 0x36: /* LD (HL), d8 */
            d8 = memory_read_8(cpu.registers.pc);
            cpu.registers.pc++;
            memory_write_8(cpu.registers.hl, d8);
            return 3;
        case 0x3E: /* LD A, d8 */
            cpu.registers.a = memory_read_8(cpu.registers.pc);
            cpu.registers.pc += 1;
            return 2;
        case 0x78: /* LD A, B */
            cpu.registers.a = cpu.registers.b;
            return 1;
        case 0xAF: /* XOR A,A */
            cpu.registers.a = 0;
            cpu.registers.f = FLAGS_Z;
            return 1;
        case 0xB1: /* OR C */
            cpu.registers.a |= cpu.registers.c;
            if (cpu.registers.a)
                cpu.registers.f = FLAGS_RST;
            else
                cpu.registers.f = FLAGS_Z;
            return 1;
        case 0xC3: /* JP a16 */
            cpu.registers.pc = memory_read_16(cpu.registers.pc);
            return 3;
        case 0xC9: /* RET */
            cpu.registers.pc = memory_read_16(cpu.registers.sp);
            cpu.registers.sp += 2;
            return 4;
        case 0xCD: /* CALL a16 */
            cpu.registers.sp -= 2;
            memory_write_16(cpu.registers.sp, cpu.registers.pc + 2);
            cpu.registers.pc = memory_read_16(cpu.registers.pc);
            return 6;
        case 0xE0: /* LD ($FF00+a8),A */
            memory_write_8(0xFF00 + memory_read_8(cpu.registers.pc), cpu.registers.a);
            cpu.registers.pc++;
            return 3;
        case 0xE2: /* ($FF00+C), A */
            memory_write_8(0xFF00 + cpu.registers.c, cpu.registers.a);
            return 2;
        case 0xEA: /* LD (a16), A */
            memory_write_8(memory_read_16(cpu.registers.pc), cpu.registers.a);
            cpu.registers.pc += 2;
            return 4;
        case 0xF0: /* LD A,($FF00+a8) */
            cpu.registers.a = memory_read_8(0xFF00 + memory_read_8(cpu.registers.pc));
            cpu.registers.pc++;
            return 3;
        case 0xF3: /* DI */
            interrupt_disable();    
            return 1;
        case 0xFE: /* CP d8 */
            cpu.registers.f = FLAGS_ALL;
            d8 = memory_read_8(cpu.registers.pc);
            cpu.registers.pc++;

            if (!(((cpu.registers.a & 0x0F) - (d8 & 0x0F)) & 0xF0))
                cpu.registers.f &= ~FLAGS_H;

            if (cpu.registers.a - d8)
                cpu.registers.f &= ~FLAGS_Z;

            if (d8 <= cpu.registers.a)
                cpu.registers.f &= ~FLAGS_C;
            return 2;
        default:
            LOG_MESG(LOG_WARN, "Instruction unimplemented: 0x%02X at pc 0x%04X", instr, (cpu.registers.pc) - 1);
            return 0;
    }
}
