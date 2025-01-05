#ifndef INPUT
#define INPUT

#include <inttypes.h>

enum input_key_e: uint32_t {
    INPUT_KEY_ESCAPE,
    INPUT_KEY_ARROW_UP, INPUT_KEY_ARROW_DOWN,
    INPUT_KEY_Z, INPUT_KEY_S, INPUT_KEY_Q, INPUT_KEY_D, INPUT_KEY_P, INPUT_KEY_L,
    INPUT_KEY_ENTER, INPUT_KEY_BACKSPACE,
    INPUT_KEY_END // Do not use
};

void input_load();
bool input_is_pressed(enum input_key_e query);
void input_run();

#endif