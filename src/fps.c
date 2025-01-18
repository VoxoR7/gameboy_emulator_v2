#include <SDL3/SDL.h>

#include "log.h"

#include "fps.h"

uint64_t ns_last = 0;

void fps_wait(uint64_t wait_ns) {
    const uint64_t current_ns = SDL_GetTicksNS();
    const uint64_t ellapsed_ns = current_ns - ns_last;

    if (ellapsed_ns >= wait_ns)
        goto end_func;

    SDL_DelayNS(wait_ns - ellapsed_ns);

end_func:
    const uint64_t after_ellapsed_ns = SDL_GetTicksNS();
    LOG_MESG(LOG_DEBUG, "ellapsed ms: %5.2f", ((double)(after_ellapsed_ns - ns_last)) / 1'000'000.0);
    ns_last = after_ellapsed_ns;
}