#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

#include "cpu_debug.h"
#include "cpu.h"

char cmd[512];
char last_cmd[512];
int64_t run = 0;
bool verbose = false;
bool display_registers = false;

static bool get_hex_number(char *command, uint16_t *number) {
    if (strlen(command) < 6)
        return false;

    if (command[0] != '0')
        return false;

    if (command[1] != 'x' && command[1] != 'X')
        return false;

    if (!((command[2] >= '0' && command[2] <= '9') || (command[2] >= 'a' && command[2] <= 'f') || (command[2] >= 'A' && command[2] <= 'F')))
        return false;
    
    if (!((command[3] >= '0' && command[3] <= '9') || (command[3] >= 'a' && command[3] <= 'f') || (command[3] >= 'A' && command[3] <= 'F')))
        return false;

    if (!((command[4] >= '0' && command[4] <= '9') || (command[4] >= 'a' && command[4] <= 'f') || (command[4] >= 'A' && command[4] <= 'F')))
        return false;

    if (!((command[5] >= '0' && command[5] <= '9') || (command[5] >= 'a' && command[5] <= 'f') || (command[5] >= 'A' && command[5] <= 'F')))
        return false;

    unsigned int n;
    sscanf(command, "0x%04x", &n);
    *number = n;
    return true;
}

static void info_cmd(char *command) {
    if (!strcmp(command, "registers") || !strcmp(command, "r")) {
        cpu_print_registers();
        return;
    }

    LOG_MESG(LOG_WARN, "Malformed command");
}

uint16_t breakpoints[2048];
uint16_t nr_breakpoints = 0;

static bool breakpoint_check(uint16_t pc) {
    for (uint16_t i = 0; i < nr_breakpoints; i++)
        if (breakpoints[i] == pc)
            return true;

    return false;
}

static void breakpoint_show() {
    for (uint16_t i = 0; i < nr_breakpoints; i++)
        printf("\tbreakpoint %"PRIu16": 0x%04X\n", i, breakpoints[i]);
    printf("Total showed breakpoint: %"PRIu16"\n", nr_breakpoints);
}

static void breakpoint_add(uint16_t addr) {
    for (uint16_t i = 0; i < nr_breakpoints; i++)
        if (breakpoints[i] == addr) {
            LOG_MESG(LOG_WARN, "breakpoint 0x%04X already exist!", addr);
            return;
        }

    breakpoints[nr_breakpoints] = addr;
    LOG_MESG(LOG_INFO, "breakpoint added at 0x%04X", breakpoints[nr_breakpoints]);
    nr_breakpoints++;
}

static void breakpoint_del(uint16_t addr) {
    for (uint16_t i = 0; i < nr_breakpoints; i++)
        if (breakpoints[i] == addr) {
            if (nr_breakpoints > 1 && i != nr_breakpoints - 1)
                breakpoints[i] = breakpoints[nr_breakpoints - 1];

            nr_breakpoints--;
            return;
        }

    LOG_MESG(LOG_WARN, "breakpoint 0x%04X couldn't be found!", addr);
    return;
}

static void breakpoint_cmd(char *command) {
    if (!strcmp(command, "show") || !strcmp(command, "s")) {
        breakpoint_show();
        return;
    }

    if (!strncmp(command, "add ", 4)) {
        uint16_t addr;
        if (!get_hex_number(command + 4, &addr)) {
            LOG_MESG(LOG_WARN, "Couldn't parse number");
            return;
        }
        breakpoint_add(addr);
        return;
    }

    if (!strncmp(command, "a ", 2)) {
        uint16_t addr;
        if (!get_hex_number(command + 2, &addr)) {
            LOG_MESG(LOG_WARN, "Couldn't parse number");
            return;
        }
        breakpoint_add(addr);
        return;
    }

    if (!strncmp(command, "del ", 4)) {
        uint16_t addr;
        if (!get_hex_number(command + 4, &addr)) {
            LOG_MESG(LOG_WARN, "Couldn't parse number");
            return;
        }
        breakpoint_del(addr);
        return;
    }

    if (!strncmp(command, "d ", 2)) {
        uint16_t addr;
        if (!get_hex_number(command + 2, &addr)) {
            LOG_MESG(LOG_WARN, "Couldn't parse number");
            return;
        }
        breakpoint_del(addr);
        return;
    }

    LOG_MESG(LOG_WARN, "Malformed command");
}

static void display_cmd(char *command) {
    if (!strcmp(command, "registers") || !strcmp(command, "r")) {
        if (display_registers)
            display_registers = false;
        else
            display_registers = true;
        return;
    }

    LOG_MESG(LOG_WARN, "Malformed command");
}

static void help() {
    printf("help\n");
    printf("\tdisplay this help\n");
    printf("[i|info] registers\n");
    printf("\tprint current state of registers\n");
    printf("[b|breakpoint] [s|show|a|add|d|del]\n");
    printf("\tmanipulate breakpoint\n");
    printf("[r|run] [number]\n");
    printf("\trun `number` instruction. If number is ommited, run indefinitely (or until a breakpoint is hit)\n");
    printf("[v|verbose]\n");
    printf("\tswitch verbose on|off\n");
    printf("[s|step]\n");
    printf("\texecute one instruction\n");
    printf("[d|display] registers\n");
    printf("\tdisplay registers at each instruction\n");
    printf("[e|exit]\n");
    printf("\texit VGE\n");
}

bool cpu_debug_run() {
    uint16_t pc = cpu_get_pc();
    if (breakpoint_check(pc)) {
        printf("breakpoint hit!\n");
        run = 0;
    }

    if (verbose)
        cpu_print_next_instr();

    if (run == -1)
        return true;

    if (run > 0) {
        run--;
        return true;
    }

    if (display_registers)
        cpu_print_registers();

    while (1) {
        printf("cpu_debug > ");
        if (fgets(cmd, sizeof(cmd), stdin) != cmd) {
            LOG_MESG(LOG_WARN, "Coudn't get command");
            continue;
        }

        cmd[strlen(cmd) - 1] = '\0'; // remove enter

        if (cmd[0] == '\0')
            strcpy(cmd, last_cmd);
        else
            strcpy(last_cmd, cmd);

        if (!strncmp(cmd, "info ", 5)) {
            info_cmd(cmd + 5);
            continue;
        }

        if (!strncmp(cmd, "i ", 2)) {
            info_cmd(cmd + 2);
            continue;
        }

        if (!strncmp(cmd, "breakpoint ", 11)) {
            breakpoint_cmd(cmd + 11);
            continue;
        }

        if (!strncmp(cmd, "b ", 2)) {
            breakpoint_cmd(cmd + 2);
            continue;
        }

        if ((!strcmp(cmd, "run")) || (!strcmp(cmd, "r"))) {
            run = -1;
            return true;
        }

        if (!strncmp(cmd, "run ", 4)) {
            if (cmd[4] < '0' && cmd[4] > '9') {
                LOG_MESG(LOG_WARN, "Malformed command");
                return true;
            }
            run = (int64_t)atol(cmd + 4);
            return true;
        }

        if (!strncmp(cmd, "r ", 2)) {
            if (cmd[2] < '0' && cmd[2] > '9') {
                LOG_MESG(LOG_WARN, "Malformed command");
                return true;
            }
            run = (int64_t)atol(cmd + 2);
            return true;
        }

        if (!strncmp(cmd, "display ", 8)) {
            display_cmd(cmd + 8);
            continue;
        }

        if (!strncmp(cmd, "d ", 2)) {
            display_cmd(cmd + 2);
            continue;
        }

        if ((!strcmp(cmd, "verbose")) || (!strcmp(cmd, "v"))) {
            if (verbose) {
                verbose = false;
                printf("switching verbose to false\n");
            } else {
                verbose = true;
                printf("switching verbose to true\n");
                cpu_print_next_instr();
            }
            continue;
        }

        if ((!strcmp(cmd, "step")) || (!strcmp(cmd, "s")))
            return true;

        if ((!strcmp(cmd, "exit")) || (!strcmp(cmd, "e")))
            return false;

        if (!strcmp(cmd, "help")) {
            help();
            continue;
        }

        LOG_MESG(LOG_WARN, "Malformed command");
    }
}
