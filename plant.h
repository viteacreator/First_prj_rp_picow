#pragma once

typedef struct {
    float gain;
    float tau;
} first_order_params_t;

typedef struct {
    float wn;
    float zeta;
    float gain;
} second_order_params_t;

typedef struct {
    float state1;
    float state2;
} second_order_state_t;

/** Step a first-order plant using Euler integration. */
float plant_first_order_step(float y, float u, const first_order_params_t *p, float dt);

/** Step a second-order plant (canonical form) using Euler integration. */
float plant_second_order_step(second_order_state_t *s, float u, const second_order_params_t *p, float dt);
