#ifndef INPUT
#define INPUT

#include <inttypes.h>

enum input_key_e: uint32_t {
    INPUT_KEY_ESCAPE,
    INPUT_KEY_ARROW_UP, INPUT_KEY_ARROW_DOWN,
    INPUT_KEY_ENTER,
    INPUT_KEY_END // Do not use
};

void input_load();
bool input_is_pressed(enum input_key_e query);

#endif