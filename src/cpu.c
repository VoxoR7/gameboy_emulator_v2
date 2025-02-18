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
FILE *f = NULL;

static void cpu_destroy() {
    fclose(f);
}

void cpu_init() {
    atexit(cpu_destroy);
    cpu_reset();
}

void cpu_reset() {
    cpu.registers.a = 0x01;
    cpu.registers.f = FLAGS_C | FLAGS_H | FLAGS_Z;
    cpu.registers.b = 0x00;
    cpu.registers.c = 0x13;
    cpu.registers.d = 0x00;
    cpu.registers.e = 0xD8;
    cpu.registers.h = 0x01;
    cpu.registers.l = 0x4D;
    cpu.registers.sp = 0xFFFE;
    cpu.registers.pc = 0x100;

    f = fopen("./debug.txt", "w");
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
    sprintf(msg, "next instruction at pc 0x%04X, instruction 0x%02X ->", cpu.registers.pc, instr);
    switch (instr) {
        case 0x00:
            LOG_MESG(LOG_DEBUG, "%s NOP", msg);
            break;
        case 0x01:
            LOG_MESG(LOG_DEBUG, "%s LD BC, d16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0x03:
            LOG_MESG(LOG_DEBUG, "%s INC BC", msg);
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
        case 0x11:
            LOG_MESG(LOG_DEBUG, "%s LD DE, d16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0x12:
            LOG_MESG(LOG_DEBUG, "%s LD (DE), A", msg);
            break;
        case 0x13:
            LOG_MESG(LOG_DEBUG, "%s INC DE", msg);
            break;
        case 0x14:
            LOG_MESG(LOG_DEBUG, "%s INC D", msg);
            break;
        case 0x16:
            LOG_MESG(LOG_DEBUG, "%s LD D, d8 (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0x18:
            LOG_MESG(LOG_DEBUG, "%s JR r8 (%d)", msg, (int8_t)memory_read_8(cpu.registers.pc + 1));
            break;
        case 0x19:
            LOG_MESG(LOG_DEBUG, "%s ADD HL, DE", msg);
            break;
        case 0x1A:
            LOG_MESG(LOG_DEBUG, "%s LD A, (DE)", msg);
            break;
        case 0x1C:
            LOG_MESG(LOG_DEBUG, "%s INC E", msg);
            break;
        case 0x20:
            LOG_MESG(LOG_DEBUG, "%s JR NZ, r8 (%d)", msg, (int8_t)memory_read_8(cpu.registers.pc + 1));
            break;
        case 0x21:
            LOG_MESG(LOG_DEBUG, "%s LD HL, d16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0x22:
            LOG_MESG(LOG_DEBUG, "%s LD (HL), A; HL++", msg);
            break;
        case 0x23:
            LOG_MESG(LOG_DEBUG, "%s INC HL", msg);
            break;
        case 0x28:
            LOG_MESG(LOG_DEBUG, "%s JR Z, r8 (%d)", msg, (int8_t)memory_read_8(cpu.registers.pc + 1));
            break;
        case 0x2C:
            LOG_MESG(LOG_DEBUG, "%s INC L", msg);
            break;
        case 0x2F:
            LOG_MESG(LOG_DEBUG, "%s CPL", msg);
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
        case 0x35:
            LOG_MESG(LOG_DEBUG, "%s DEC (HL)", msg);
            break;
        case 0x36:
            LOG_MESG(LOG_DEBUG, "%s LD (HL), d8 (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0x37:
            LOG_MESG(LOG_DEBUG, "%s SCF", msg);
            break;
        case 0x3E:
            LOG_MESG(LOG_DEBUG, "%s LD A, d8 (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0x47:
            LOG_MESG(LOG_DEBUG, "%s LD B, A", msg);
            break;
        case 0x4F:
            LOG_MESG(LOG_DEBUG, "%s LD C, A", msg);
            break;
        case 0x56:
            LOG_MESG(LOG_DEBUG, "%s LD D, (HL)", msg);
            break;
        case 0x5E:
            LOG_MESG(LOG_DEBUG, "%s LD E, (HL)", msg);
            break;
        case 0x5F:
            LOG_MESG(LOG_DEBUG, "%s LD E, A", msg);
            break;
        case 0x77:
            LOG_MESG(LOG_DEBUG, "%s LD (HL), A", msg);
            break;
        case 0x78:
            LOG_MESG(LOG_DEBUG, "%s LD A, B", msg);
            break;
        case 0x79:
            LOG_MESG(LOG_DEBUG, "%s LD A, C", msg);
            break;
        case 0x7C:
            LOG_MESG(LOG_DEBUG, "%s LD A, H", msg);
            break;
        case 0x7D:
            LOG_MESG(LOG_DEBUG, "%s LD A, L", msg);
            break;
        case 0x7E:
            LOG_MESG(LOG_DEBUG, "%s LD A, (HL)", msg);
            break;
        case 0x87:
            LOG_MESG(LOG_DEBUG, "%s ADD A, A", msg);
            break;
        case 0xA1:
            LOG_MESG(LOG_DEBUG, "%s AND A, C", msg);
            break;
        case 0xA7:
            LOG_MESG(LOG_DEBUG, "%s AND A, A", msg);
            break;
        case 0xA9:
            LOG_MESG(LOG_DEBUG, "%s XOR A, C", msg);
            break;
        case 0xAF:
            LOG_MESG(LOG_DEBUG, "%s XOR A, A", msg);
            break;
        case 0xB0:
            LOG_MESG(LOG_DEBUG, "%s OR B", msg);
            break;
        case 0xB1:
            LOG_MESG(LOG_DEBUG, "%s OR C", msg);
            break;
        case 0xC1:
            LOG_MESG(LOG_DEBUG, "%s POP BC", msg);
            break;
        case 0xC3:
            LOG_MESG(LOG_DEBUG, "%s JP a16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0xC4:
            LOG_MESG(LOG_DEBUG, "%s CALL NZ a16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0xC5:
            LOG_MESG(LOG_DEBUG, "%s PUSH BC", msg);
            break;
        case 0xC9:
            LOG_MESG(LOG_DEBUG, "%s RET", msg);
            break;
        case 0xCA:
            LOG_MESG(LOG_DEBUG, "%s JP Z, a16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0xCB:
            LOG_MESG(LOG_DEBUG, "%s LD PREFIX CB (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0xCD:
            LOG_MESG(LOG_DEBUG, "%s CALL a16 (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0xD1:
            LOG_MESG(LOG_DEBUG, "%s POP DE", msg);
            break;
        case 0xD5:
            LOG_MESG(LOG_DEBUG, "%s PUSH DE", msg);
            break;
        case 0xE0:
            LOG_MESG(LOG_DEBUG, "%s LD ($FF00+a8), A (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0xE1:
            LOG_MESG(LOG_DEBUG, "%s POP HL", msg);
            break;
        case 0xE2:
            LOG_MESG(LOG_DEBUG, "%s ($FF00+C), A", msg);
            break;
        case 0xE5:
            LOG_MESG(LOG_DEBUG, "%s PUSH HL", msg);
            break;
        case 0xE6:
            LOG_MESG(LOG_DEBUG, "%s AND d8", msg);
            break;
        case 0xE9:
            LOG_MESG(LOG_DEBUG, "%s JP HL", msg);
            break;
        case 0xEA:
            LOG_MESG(LOG_DEBUG, "%s LD (a16), A (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0xEF:
            LOG_MESG(LOG_DEBUG, "%s RST 028H", msg);
            break;
        case 0xF0:
            LOG_MESG(LOG_DEBUG, "%s LD A, ($FF00+a8) (0x%02X)", msg, memory_read_8(cpu.registers.pc + 1));
            break;
        case 0xF1:
            LOG_MESG(LOG_DEBUG, "%s POP AF", msg);
            break;
        case 0xF3:
            LOG_MESG(LOG_DEBUG, "%s DI", msg);
            break;
        case 0xF5:
            LOG_MESG(LOG_DEBUG, "%s PUSH AF", msg);
            break;
        case 0xFA:
            LOG_MESG(LOG_DEBUG, "%s LD A, (a16) (0x%04X)", msg, memory_read_16(cpu.registers.pc + 1));
            break;
        case 0xFB:
            LOG_MESG(LOG_DEBUG, "%s EI", msg);
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

static uint8_t cpu_execute_prefix_cb() {
    uint8_t instr = memory_read_8(cpu.registers.pc);
    cpu.registers.pc++;

    switch (instr) {
        case 0x37: /* SWAP A */
            if (!cpu.registers.a) {
                cpu.registers.f = FLAGS_Z;
                return 2;
            }

            cpu.registers.f = FLAGS_RST;

            uint8_t a = cpu.registers.a;
            cpu.registers.a = ((a & 0x0F) << 4) + ((a & 0xF0) >> 4);
            return 2;
        case 0x87: /* RES 0, A */
            cpu.registers.a &= 0b1111'1110;
            return 2;
        default:
            LOG_MESG(LOG_WARN, "PREFIX CB unimplemented: 0x%02X at pc 0x%04X", instr, (cpu.registers.pc) - 1);
            return 0;
    }
}

uint8_t cpu_execute() {
    fprintf(
        f,
        "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n",
        cpu.registers.a,
        cpu.registers.f,
        cpu.registers.b,
        cpu.registers.c,
        cpu.registers.d,
        cpu.registers.e,
        cpu.registers.h,
        cpu.registers.l,
        cpu.registers.sp,
        cpu.registers.pc,
        memory_read_8(cpu.registers.pc),
        memory_read_8(cpu.registers.pc + 1),
        memory_read_8(cpu.registers.pc + 2),
        memory_read_8(cpu.registers.pc + 3)
    );
    fflush(f);

    uint8_t instr = memory_read_8(cpu.registers.pc);
    // LOG_MESG(LOG_DEBUG, "Instruction 0x%02X (d8 0x%02X, d16 0x%04X) at pc 0x%04X", instr, memory_read_8(cpu.registers.pc), memory_read_16(cpu.registers.pc), cpu.registers.pc);
    cpu.registers.pc++;

    uint8_t d8;
    int8_t r8;
    uint16_t a16;

    switch (instr) {
        case 0x00: /* NOP */
            return 1;
        case 0x01: /* LD BC, d16 */
            cpu.registers.bc = memory_read_16(cpu.registers.pc);
            cpu.registers.pc += 2;
            return 3;
        case 0x03: /* INC BC */
            cpu.registers.bc++;
            return 2;
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
        case 0x11: /* LD DE, d16 */
            cpu.registers.de = memory_read_16(cpu.registers.pc);
            cpu.registers.pc += 2;
            return 3;
        case 0x12: /* LD (DE), A */
            memory_write_8(cpu.registers.de, cpu.registers.a);
            return 2;
        case 0x13: /* INC DE */
            cpu.registers.de++;
            return 2;
        case 0x14: /* INC D */
            cpu.registers.f &= ~FLAGS_N;
            if (((cpu.registers.d & 0x0F) + 1) & 0xF0)
                cpu.registers.f |= FLAGS_H;
            else
                cpu.registers.f &= ~FLAGS_H;
            cpu.registers.d++;
            if (!cpu.registers.d)
                cpu.registers.f |= FLAGS_Z;
            else
                cpu.registers.f &= ~FLAGS_Z;
            return 1;
        case 0x16: /* LD D, d8 */
            cpu.registers.d = memory_read_8(cpu.registers.pc);
            cpu.registers.pc += 1;
            return 2;
        case 0x18: /* JR r8 */
            r8 = (int8_t)memory_read_8(cpu.registers.pc);
            cpu.registers.pc++;
            cpu.registers.pc += r8;
            return 3;
        case 0x19: /* ADD HL, DE */
            cpu.registers.f &= ~FLAGS_N;
            if (((cpu.registers.hl & 0x0FFF) + (cpu.registers.de & 0x0FFF)) & 0xF000)
                cpu.registers.f |= FLAGS_H;
            else
                cpu.registers.f &= ~FLAGS_H;

            if ((uint64_t)cpu.registers.hl + (uint64_t)cpu.registers.de > UINT16_MAX)
                cpu.registers.f |= FLAGS_C;
            else
                cpu.registers.f &= ~FLAGS_C;

            cpu.registers.hl += cpu.registers.de;
            return 2;
        case 0x1A: /* LD A, (DE) */
            cpu.registers.a = memory_read_8(cpu.registers.de);
            return 2;
        case 0x1C: /* INC E */
            cpu.registers.f &= ~FLAGS_N;
            if (((cpu.registers.e & 0x0F) + 1) & 0xF0)
                cpu.registers.f |= FLAGS_H;
            else
                cpu.registers.f &= ~FLAGS_H;
            cpu.registers.e++;
            if (!cpu.registers.e)
                cpu.registers.f |= FLAGS_Z;
            else
                cpu.registers.f &= ~FLAGS_Z;
            return 1;
        case 0x20: /* JR NZ, r8 */
            r8 = (int8_t)memory_read_8(cpu.registers.pc);
            cpu.registers.pc++;
            if (cpu.registers.f & FLAGS_Z)
                return 2;

            cpu.registers.pc += r8;
            return 3;
        case 0x21: /* LD HL, d16 */
            cpu.registers.hl = memory_read_16(cpu.registers.pc);
            cpu.registers.pc += 2;
            return 3;
        case 0x22: /* LD (HL), A; HL++ */
            memory_write_8(cpu.registers.hl, cpu.registers.a);
            cpu.registers.hl++;
            return 2;
        case 0x23: /* INC HL */
            cpu.registers.hl++;
            return 2;
        case 0x28: /* JR Z, r8 */
            r8 = (int8_t)memory_read_8(cpu.registers.pc);
            cpu.registers.pc++;
            if (!(cpu.registers.f & FLAGS_Z))
                return 2;

            cpu.registers.pc += r8;
            return 3;
        case 0x2A: /* LD A, (HL); INC HL */
            cpu.registers.a = memory_read_8(cpu.registers.hl);
            cpu.registers.hl++;
            return 2;
        case 0x2C: /* INC L */
            cpu.registers.f &= ~FLAGS_N;

            if (((cpu.registers.l & 0x0F) + 1) & 0xF0)
                cpu.registers.f |= FLAGS_H;
            else
                cpu.registers.f &= ~FLAGS_H;

            cpu.registers.l++;

            if (cpu.registers.l)
                cpu.registers.f &= ~FLAGS_Z;
            else
                cpu.registers.l |= FLAGS_Z;
            return 1;
        case 0x2F: /* CPL */
            cpu.registers.f |= FLAGS_N | FLAGS_H;
            cpu.registers.a = ~cpu.registers.a;
            return 1;
        case 0x31: /* LD SP, d16 */
            cpu.registers.sp = memory_read_16(cpu.registers.pc);
            cpu.registers.pc += 2;
            return 3;
        case 0x32: /* LD (HL), A; DEC HL */
            memory_write_8(cpu.registers.hl, cpu.registers.a);
            cpu.registers.hl--;
            return 2;
        case 0x35: /* DEC (HL) */
            cpu.registers.f |= FLAGS_N;
            uint8_t val = memory_read_8(cpu.registers.hl);
            if (((val & 0xF) - 1) & 0xF0)
                cpu.registers.f |= FLAGS_H;
            else
                cpu.registers.f &= ~FLAGS_H;

            val--;

            if (val)
                cpu.registers.f &= ~FLAGS_Z;
            else
                cpu.registers.f |= FLAGS_Z;

            memory_write_8(cpu.registers.hl, val);
            return 3;
        case 0x36: /* LD (HL), d8 */
            d8 = memory_read_8(cpu.registers.pc);
            cpu.registers.pc++;
            memory_write_8(cpu.registers.hl, d8);
            return 3;
        case 0x37: /* SCF */
            cpu.registers.f &= ~(FLAGS_N | FLAGS_H);
            cpu.registers.f |= FLAGS_C;
            return 1;
        case 0x3E: /* LD A, d8 */
            cpu.registers.a = memory_read_8(cpu.registers.pc);
            cpu.registers.pc += 1;
            return 2;
        case 0x47: /* LD B, A */
            cpu.registers.b = cpu.registers.a;
            return 1;
        case 0x4F: /* LD C, A */
            cpu.registers.c = cpu.registers.a;
            return 1;
        case 0x56: /* LD D, (HL) */
            cpu.registers.d = memory_read_8(cpu.registers.hl);
            return 2;
        case 0x5E: /* LD E, (HL) */
            cpu.registers.e = memory_read_8(cpu.registers.hl);
            return 2;
        case 0x5F: /* LD E, A */
            cpu.registers.e = cpu.registers.a;
            return 1;
        case 0x77: /* LD (HL), A */
            memory_write_8(cpu.registers.hl, cpu.registers.a);
            return 2;
        case 0x78: /* LD A, B */
            cpu.registers.a = cpu.registers.b;
            return 1;
        case 0x79: /* LD A, C */
            cpu.registers.a = cpu.registers.c;
            return 1;
        case 0x7C: /* LD A, H */
            cpu.registers.a = cpu.registers.h;
            return 1;
        case 0x7D: /* LD A, L */
            cpu.registers.a = cpu.registers.l;
            return 1;
        case 0x7E: /* LD A, (HL) */
            cpu.registers.a = memory_read_8(cpu.registers.hl);
            return 2;
        case 0x87: /* ADD A, A */
            cpu.registers.f = FLAGS_RST;
            if (((cpu.registers.a & 0x0F) + (cpu.registers.a & 0x0F)) & 0xF0)
                cpu.registers.f |= FLAGS_H;
            if (cpu.registers.a >= 128)
                cpu.registers.f |= FLAGS_C;

            cpu.registers.a *= 2;

            if (!cpu.registers.a)
                cpu.registers.f |= FLAGS_Z;
            return 1;
        case 0xA1: /* AND A, C */
            cpu.registers.a &= cpu.registers.c;
            if (cpu.registers.a)
                cpu.registers.f = FLAGS_H;
            else
                cpu.registers.f = FLAGS_Z | FLAGS_H;
            return 1;
        case 0xA7: /* AND A, A */
            cpu.registers.f = FLAGS_H;
            if (!cpu.registers.a)
                cpu.registers.f |= FLAGS_Z;
            return 1;
        case 0xA9: /* XOR A, C */
            cpu.registers.a ^= cpu.registers.c;
            if (cpu.registers.a)
                cpu.registers.f = FLAGS_RST;
            else
                cpu.registers.f = FLAGS_Z;
            return 1;
        case 0xAF: /* XOR A, A */
            cpu.registers.a = 0;
            cpu.registers.f = FLAGS_Z;
            return 1;
        case 0xB0: /* OR B */
            cpu.registers.a |= cpu.registers.b;
            if (cpu.registers.a)
                cpu.registers.f = FLAGS_RST;
            else
                cpu.registers.f = FLAGS_Z;
            return 1;
        case 0xB1: /* OR C */
            cpu.registers.a |= cpu.registers.c;
            if (cpu.registers.a)
                cpu.registers.f = FLAGS_RST;
            else
                cpu.registers.f = FLAGS_Z;
            return 1;
        case 0xC1: /* POP BC */
            cpu.registers.bc = memory_read_16(cpu.registers.sp);
            cpu.registers.sp += 2;
            return 3;
        case 0xC3: /* JP a16 */
            cpu.registers.pc = memory_read_16(cpu.registers.pc);
            return 3;
        case 0xC4: /* CALL NZ a16 */
            a16 = memory_read_16(cpu.registers.pc);
            if (cpu.registers.f & FLAGS_Z) {
                cpu.registers.pc += 2;
                return 3;
            }

            cpu.registers.sp -= 2;
            memory_write_16(cpu.registers.sp, cpu.registers.pc + 2);
            cpu.registers.pc = a16;
            return 6;
        case 0xC5: /* PUSH BC */
            cpu.registers.sp -= 2;
            memory_write_16(cpu.registers.sp, cpu.registers.bc);
            return 4;
        case 0xC9: /* RET */
            cpu.registers.pc = memory_read_16(cpu.registers.sp);
            cpu.registers.sp += 2;
            return 4;
        case 0xCA: /* JP Z, a16 */
            a16 = memory_read_16(cpu.registers.pc);
            cpu.registers.pc += 2;
            if (!(cpu.registers.f & FLAGS_Z))
                return 3;

            cpu.registers.pc = a16;
            return 4;
        case 0xCB: /* PREFIX CB */
            return cpu_execute_prefix_cb();
        case 0xCD: /* CALL a16 */
            cpu.registers.sp -= 2;
            memory_write_16(cpu.registers.sp, cpu.registers.pc + 2);
            cpu.registers.pc = memory_read_16(cpu.registers.pc);
            return 6;
        case 0xD1: /* POP DE */
            cpu.registers.de = memory_read_16(cpu.registers.sp);
            cpu.registers.sp += 2;
            return 3;
        case 0xD5: /* PUSH DE */
            cpu.registers.sp -= 2;
            memory_write_16(cpu.registers.sp, cpu.registers.de);
            return 4;
        case 0xE0: /* LD ($FF00+a8),A */
            memory_write_8(0xFF00 + memory_read_8(cpu.registers.pc), cpu.registers.a);
            cpu.registers.pc++;
            return 3;
        case 0xE1: /* POP HL */
            cpu.registers.hl = memory_read_16(cpu.registers.sp);
            cpu.registers.sp += 2;
            return 3;
        case 0xE2: /* ($FF00+C), A */
            memory_write_8(0xFF00 + cpu.registers.c, cpu.registers.a);
            return 2;
        case 0xE5: /* PUSH HL */
            cpu.registers.sp -= 2;
            memory_write_16(cpu.registers.sp, cpu.registers.hl);
            return 4;
        case 0xE6: /* AND d8 */
            cpu.registers.f = FLAGS_H;
            d8 = memory_read_8(cpu.registers.pc);
            cpu.registers.pc++;
            cpu.registers.a &= d8;
            if (!cpu.registers.a)
                cpu.registers.f |= FLAGS_Z;
            return 2;
        case 0xE9: /* JP HL */
            cpu.registers.pc = cpu.registers.hl;
            return 1;
        case 0xEA: /* LD (a16), A */
            memory_write_8(memory_read_16(cpu.registers.pc), cpu.registers.a);
            cpu.registers.pc += 2;
            return 4;
        case 0xEF: /* RST 028H */
            cpu.registers.sp -= 2;
            memory_write_16(cpu.registers.sp, cpu.registers.pc);
            cpu.registers.pc = 0x28;
            return 4;
        case 0xF0: /* LD A,($FF00+a8) */
            cpu.registers.a = memory_read_8(0xFF00 + memory_read_8(cpu.registers.pc));
            cpu.registers.pc++;
            return 3;
        case 0xF1: /* POP AF */
            cpu.registers.af = memory_read_16(cpu.registers.sp);
            cpu.registers.sp += 2;
            return 3;
        case 0xF3: /* DI */
            interrupt_disable();    
            return 1;
        case 0xF5: /* PUSH AF */
            cpu.registers.sp -= 2;
            memory_write_16(cpu.registers.sp, cpu.registers.af);
            return 4;
        case 0xFA: /* LD A, (a16) */
            cpu.registers.a = memory_read_8(cpu.registers.pc);
            cpu.registers.pc += 2;
            return 4;
        case 0xFB: /* EI */
            interrupt_enable();
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
