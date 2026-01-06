#include "pid.h"

/** Initialize PID with gains and output limits. */
void pid_init(pid_t *pid, float kp, float ki, float kd, float out_min, float out_max) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integrator = 0.0f;
    pid->prev_error = 0.0f;
    pid->out_min = out_min;
    pid->out_max = out_max;
}

/** Reset internal PID state (integrator and previous error). */
void pid_reset(pid_t *pid) {
    pid->integrator = 0.0f;
    pid->prev_error = 0.0f;
}

/** Compute PID output for the given error and time step. */
float pid_step(pid_t *pid, float error, float dt) {
    float derivative = (dt > 0.0f) ? (error - pid->prev_error) / dt : 0.0f;
    pid->integrator += error * dt;

    float out = (pid->kp * error) + (pid->ki * pid->integrator) + (pid->kd * derivative);

    /* Clamp only when limits are enabled (min <= max). */
    if (pid->out_min <= pid->out_max) {
        if (out > pid->out_max) out = pid->out_max;
        if (out < pid->out_min) out = pid->out_min;
    }

    pid->prev_error = error;
    return out;
}
