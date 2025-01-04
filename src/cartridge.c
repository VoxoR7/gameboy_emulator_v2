#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

#include "cartridge.h"

#define MAX_BANK 32

struct cartridge_s {
    uint8_t bank_loaded;
    uint8_t *(banks[MAX_BANK]);
};

struct cartridge_s *cartridge_load(char *path) {
    struct cartridge_s *cartridge = malloc(sizeof(struct cartridge_s));
    if (!cartridge) {
        LOG_MESG(LOG_WARN, "Couldn't malloc");
        return NULL;
    }
    cartridge->bank_loaded = 0;

    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        LOG_MESG(LOG_WARN, "Couldn't open file %s", path);
        return NULL;
    }

    uint8_t bank[BANK_SIZE];
    size_t nr_read;

    do {
        nr_read = fread(bank, 1, BANK_SIZE, f);
        if (nr_read == 0)
            continue;

        if (nr_read > 0 && nr_read != BANK_SIZE) {
            if (cartridge->bank_loaded == 0) {
                LOG_MESG(LOG_WARN, "Couldn't load any bank (nr read: %lu)", nr_read);
                fclose(f);
                free(cartridge);
                return NULL;
            }

            LOG_MESG(LOG_WARN, "bank #%"PRIu8" couldn't be load properly, not included (nr read: %lu)", cartridge->bank_loaded, nr_read);
        }

        cartridge->banks[cartridge->bank_loaded] = malloc(BANK_SIZE);
        if (!cartridge->banks[cartridge->bank_loaded]) {
            LOG_MESG(LOG_WARN, "Couldn't malloc");
            cartridge_unload(cartridge);
            return NULL;
        }

        memcpy(cartridge->banks[cartridge->bank_loaded], bank, BANK_SIZE);

        cartridge->bank_loaded++;
    } while (nr_read == BANK_SIZE);

    LOG_MESG(LOG_DEBUG, "Could load %"PRIu8" banks succesfully", cartridge->bank_loaded);
    return cartridge;
}

void cartridge_unload(struct cartridge_s *cartridge) {
    for (uint8_t bank = 0; bank < cartridge->bank_loaded; bank++)
        free(cartridge->banks[bank]);

    free(cartridge);
}

uint8_t *cartridge_get_bank(struct cartridge_s *cartridge, uint8_t bank) {
    if (cartridge->bank_loaded <= bank) {
        LOG_MESG(LOG_WARN, "Asked for bank %"PRIu8" but the cartridge contain only %"PRIu8" banks", bank, cartridge->bank_loaded);
        return NULL;
    }

    return cartridge->banks[bank];
}
