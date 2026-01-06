#include <string.h>

#include "sim_state.h"

sim_state_t g_sim;

/** Initialize shared simulation state with safe defaults. */
void sim_state_init(void) {
    critical_section_init(&g_sim.lock);
    g_sim.cfg.setpoint = 200.0f;
    g_sim.cfg.pid.kp = 2.0f;
    g_sim.cfg.pid.ki = 0.5f;
    g_sim.cfg.pid.kd = 0.1f;
    g_sim.cfg.plant.model = PLANT_FIRST_ORDER;
    g_sim.cfg.plant.gain = 2.0f;
    g_sim.cfg.plant.tau = 8.0f;
    g_sim.cfg.plant.wn = 1.2f;
    g_sim.cfg.plant.zeta = 0.7f;
    g_sim.cfg.plant.dead_time_ms = 0.0f;
    g_sim.cfg.act_inject = 1;
    g_sim.cfg.act_absorb = 1;
    g_sim.cfg.running = 0;
    g_sim.rt.time_s = 0.0f;
    g_sim.rt.setpoint = g_sim.cfg.setpoint;
    g_sim.rt.control = 0.0f;
    g_sim.rt.actuator = 0.0f;
    g_sim.rt.output = 25.0f;
    g_sim.reset_requested = 0;
}
