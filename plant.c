#include "plant.h"

/** Step a first-order plant using Euler integration. */
float plant_first_order_step(float y, float u, const first_order_params_t *p, float dt) {
    float tau = p->tau;
    if (tau < 0.001f) tau = 0.001f;
    float dy = (-y + p->gain * u) / tau;
    return y + dy * dt;
}

/** Step a second-order plant (canonical form) using Euler integration. */
float plant_second_order_step(second_order_state_t *s, float u, const second_order_params_t *p, float dt) {
    float x1 = s->state1;
    float x2 = s->state2;

    float dx1 = x2;
    float wn = p->wn;
    if (wn < 0.001f) wn = 0.001f;
    float zeta = p->zeta;
    if (zeta < 0.0f) zeta = 0.0f;
    float wn2 = wn * wn;
    float dx2 = (p->gain * wn2 * u) - (2.0f * zeta * wn * x2) - (wn2 * x1);

    x1 += dx1 * dt;
    x2 += dx2 * dt;

    s->state1 = x1;
    s->state2 = x2;
    return x1;
}
