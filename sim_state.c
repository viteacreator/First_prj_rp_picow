#include <string.h>

#include "sim_state.h"

sim_state_t g_sim;

/** Initialize shared simulation state with safe defaults. */
void sim_state_init(void) {
    critical_section_init(&g_sim.lock);
    g_sim.cfg.setpoint = 200.0f;
    g_sim.cfg.master_setpoint = 0.0f;
    g_sim.cfg.use_master_setpoint = 0;
    g_sim.cfg.allow_sens_signal = 1;
    g_sim.cfg.dt_ms = 10;
    g_sim.cfg.pid.kp = 2.0f;
    g_sim.cfg.pid.ki = 0.5f;
    g_sim.cfg.pid.kd = 0.1f;
    g_sim.cfg.plant.model = PLANT_FIRST_ORDER;
    g_sim.cfg.plant.gain = 2.0f;
    g_sim.cfg.plant.tau = 8.0f;
    g_sim.cfg.plant.wn = 1.2f;
    g_sim.cfg.plant.zeta = 0.7f;
    g_sim.cfg.plant.dead_time_ms = 0;
    g_sim.cfg.act_inject = 1;
    g_sim.cfg.act_absorb = 1;
    g_sim.cfg.act_min = -100.0f;
    g_sim.cfg.act_max = 100.0f;
    g_sim.cfg.running = 0;
    g_sim.rt.time_s = 0.0f;
    g_sim.rt.setpoint = g_sim.cfg.setpoint;
    g_sim.rt.control = 0.0f;
    g_sim.rt.actuator = 0.0f;
    g_sim.rt.output = 25.0f;
    g_sim.reset_requested = 0;
}

/** Set operator (UI) setpoint value. */
void sim_state_set_setpoint(float setpoint) {
    critical_section_enter_blocking(&g_sim.lock);
    g_sim.cfg.setpoint = setpoint;
    critical_section_exit(&g_sim.lock);
}

/** Read operator (UI) setpoint value. */
float sim_state_get_setpoint(void) {
    float setpoint;
    critical_section_enter_blocking(&g_sim.lock);
    setpoint = g_sim.cfg.setpoint;
    critical_section_exit(&g_sim.lock);
    return setpoint;
}

/** Set master setpoint value from external controller. */
void master_setpoint_set(float m_setpoint) {
    critical_section_enter_blocking(&g_sim.lock);
    g_sim.cfg.master_setpoint = m_setpoint;
    critical_section_exit(&g_sim.lock);
}

/** Read master setpoint value. */
float master_setpoint_get(void) {
    float setpoint;
    critical_section_enter_blocking(&g_sim.lock);
    setpoint = g_sim.cfg.master_setpoint;
    critical_section_exit(&g_sim.lock);
    return setpoint;
}

/** Select whether the master setpoint drives the control loop. */
void sim_state_set_use_master_setpoint(int use_master) {
    critical_section_enter_blocking(&g_sim.lock);
    g_sim.cfg.use_master_setpoint = use_master ? 1 : 0;
    critical_section_exit(&g_sim.lock);
}

/** Read whether master setpoint drives the control loop. */
int sim_state_get_use_master_setpoint(void) {
    int use_master;
    critical_section_enter_blocking(&g_sim.lock);
    use_master = g_sim.cfg.use_master_setpoint;
    critical_section_exit(&g_sim.lock);
    return use_master;
}

/** Enable/disable sensor feedback signal in the control loop. */
void sim_state_set_allow_sens_signal(int allow) {
    critical_section_enter_blocking(&g_sim.lock);
    g_sim.cfg.allow_sens_signal = allow ? 1 : 0;
    critical_section_exit(&g_sim.lock);
}

/** Read whether sensor feedback is enabled. */
int sim_state_get_allow_sens_signal(void) {
    int allow;
    critical_section_enter_blocking(&g_sim.lock);
    allow = g_sim.cfg.allow_sens_signal;
    critical_section_exit(&g_sim.lock);
    return allow;
}

/** Update PID parameters from external controller. */
void sim_state_set_pid(const pid_params_t *pid) {
    if (!pid) return;
    critical_section_enter_blocking(&g_sim.lock);
    g_sim.cfg.pid = *pid;
    critical_section_exit(&g_sim.lock);
}

/** Read current PID parameters. */
pid_params_t sim_state_get_pid(void) {
    pid_params_t pid;
    critical_section_enter_blocking(&g_sim.lock);
    pid = g_sim.cfg.pid;
    critical_section_exit(&g_sim.lock);
    return pid;
}
