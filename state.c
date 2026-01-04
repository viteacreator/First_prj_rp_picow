#include <string.h>

#include "state.h"

shared_state_t g_state;

/* Initialize shared state with safe defaults. */
void state_init(void) {
    critical_section_init(&g_state.lock);
    g_state.prime_max = 5000;
    g_state.prime_count = 0;
    g_state.last_prime = 0;
    g_state.led_manual = 0;
    g_state.blink_ms = 0;
    g_state.last_msg[0] = '\0';
}
