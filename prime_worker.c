#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "state.h"

/* Check if a number is prime using a simple trial division. */
static bool is_prime(uint32_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if ((n & 1u) == 0u) return false;
    for (uint32_t i = 3; i * i <= n; i += 2) {
        if ((n % i) == 0u) return false;
    }
    return true;
}

/* Core 1 entry: continuously count primes and update shared stats. */
static void core1_main(void) {
    uint32_t n = 2;
    while (true) {
        uint32_t local_max;
        critical_section_enter_blocking(&g_state.lock);
        local_max = g_state.prime_max;
        critical_section_exit(&g_state.lock);

        if (is_prime(n)) {
            critical_section_enter_blocking(&g_state.lock);
            g_state.prime_count++;
            g_state.last_prime = n;
            critical_section_exit(&g_state.lock);
        }

        n++;
        if (n > local_max) {
            n = 2;
        }
        tight_loop_contents();
    }
}

/* Launch the core1 worker so core0 can handle Wi-Fi and UI. */
void prime_worker_start(void) {
    multicore_launch_core1(core1_main);
}
