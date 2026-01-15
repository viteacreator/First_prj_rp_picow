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
    int use_master_setpoint; // flag: use master setpoint if non-zero
    int allow_sens_signal; // flag: allow sensor feedback if non-zero
    int dt_ms; // Simulation time step in milliseconds
    pid_params_t pid;
    plant_params_t plant;
    int act_inject; // flag: actuator inject mode if non-zero
    int act_absorb; // flag: actuator absorb mode if non-zero
    float act_min;
    float act_max;
    int running; // flag: simulation running if non-zero
} sim_config_t;

typedef struct {
    float time_s; // Elapsed simulation time in seconds (t)
    float setpoint; // Current active setpoint r(t)
    float control; // Current controller output u(t)
    float actuator; // Current actuator value after limits u1(t)
    float output; // Current plant output y(1)
} sim_runtime_t;

typedef struct {
    critical_section_t lock; // iteresting lock for thread-safe access
    sim_config_t cfg; // simulation configuration parameters
    sim_runtime_t rt; // real-time simulation data
    int reset_requested; // flag: reset requested by external controller
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

/** 
 * Select whether to use master setpoint as active reference. 
 * */
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
