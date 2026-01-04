#pragma once

#include <stdint.h>
#include "pico/sync.h"

#define MAX_MSG_LEN 96

typedef struct {
    critical_section_t lock;
    uint32_t prime_count;
    uint32_t last_prime;
    uint32_t prime_max;
    int led_manual;
    int blink_ms;
    char last_msg[MAX_MSG_LEN];
} shared_state_t;

extern shared_state_t g_state;

/* Initialize shared state and its lock. */
void state_init(void);
