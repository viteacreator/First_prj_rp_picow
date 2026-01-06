#pragma once

#include <stdint.h>
#include "pico/sync.h"

typedef enum {
    PLANT_FIRST_ORDER = 0,
    PLANT_SECOND_ORDER = 1
} plant_model_t;

typedef struct {
    float kp;
    float ki;
    float kd;
} pid_params_t;

typedef struct {
    plant_model_t model;
    float gain;
    float tau;
    float wn;
    float zeta;
    float dead_time_ms;
} plant_params_t;

typedef struct {
    float setpoint;
    pid_params_t pid;
    plant_params_t plant;
    int act_inject;
    int act_absorb;
    int running;
} sim_config_t;

typedef struct {
    float time_s;
    float setpoint;
    float control;
    float actuator;
    float output;
} sim_runtime_t;

typedef struct {
    critical_section_t lock;
    sim_config_t cfg;
    sim_runtime_t rt;
    int reset_requested;
} sim_state_t;

extern sim_state_t g_sim;

/** Initialize shared simulation state and its lock. */
void sim_state_init(void);
