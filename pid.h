#pragma once

typedef struct {
    float kp;
    float ki;
    float kd;
    float integrator;
    float prev_error;
    float out_min;
    float out_max;
} pid_t;

/** Initialize PID with gains and output limits. */
void pid_init(pid_t *pid, float kp, float ki, float kd, float out_min, float out_max);

/** Reset internal PID state (integrator and previous error). */
void pid_reset(pid_t *pid);

/** Compute PID output for the given error and time step. */
float pid_step(pid_t *pid, float error, float dt);
