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
    int dead_time_ms;
} plant_params_t;

typedef struct {
    float setpoint;
    float master_setpoint;
    int use_master_setpoint;
    int allow_sens_signal;
    int dt_ms;
    pid_params_t pid;
    plant_params_t plant;
    int act_inject;
    int act_absorb;
    float act_min;
    float act_max;
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
/** Set operator (UI) setpoint. */
void sim_state_set_setpoint(float setpoint);
/** Get operator (UI) setpoint. */
float sim_state_get_setpoint(void);
/** Set master setpoint from external controller. */
void master_setpoint_set(float m_setpoint);
/** Get master setpoint from external controller. */
float master_setpoint_get(void);
/** Select whether to use master setpoint as active reference. */
void sim_state_set_use_master_setpoint(int use_master);
/** Get whether master setpoint is active. */
int sim_state_get_use_master_setpoint(void);
/** Enable/disable sensor feedback signal in the control loop. */
void sim_state_set_allow_sens_signal(int allow);
/** Get whether sensor feedback is enabled. */
int sim_state_get_allow_sens_signal(void);
/** Update PID parameters from external controller. */
void sim_state_set_pid(const pid_params_t *pid);
/** Read current PID parameters. */
pid_params_t sim_state_get_pid(void);
