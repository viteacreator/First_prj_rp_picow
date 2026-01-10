#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "pid.h"
#include "plant.h"
#include "sim_state.h"
#include "debug.h"

#define DEFAULT_DT_MS 10
#define DEAD_TIME_BUFFER 256

/** Map controller output into actuator output based on mode and limits. */
static float actuator_apply(float u, int inject, int absorb, float min_out, float max_out) {
    /* Disabled actuator: no effect on plant. */
    if (!inject && !absorb) {
        return 0.0f;
    }

    /* Force limits to be sane. */
    if (min_out > max_out) {
        float tmp = min_out;
        min_out = max_out;
        max_out = tmp;
    }

    /* Inject-only: disallow negative output. */
    if (inject && !absorb) {
        if (min_out < 0.0f) min_out = 0.0f;
        if (max_out < 0.0f) max_out = 0.0f;
    }

    /* Absorb-only: disallow positive output. */
    if (!inject && absorb) {
        if (max_out > 0.0f) max_out = 0.0f;
        if (min_out > 0.0f) min_out = 0.0f;
    }

    if (min_out > max_out) {
        min_out = max_out;
    }

    if (u < min_out) return min_out;
    if (u > max_out) return max_out;
    return u;
}

/** Core 1 entry: simulate plant dynamics and apply PID in real time. */
static void core1_main(void) {
    pid_t pid;
    /* PID is unconstrained; actuator applies the physical limits. */
    pid_init(&pid, 2.0f, 0.5f, 0.1f, 1.0f, -1.0f);

    second_order_state_t second_state = {0};

    float y = 25.0f;
    float u = 0.0f;
    float u1 = 0.0f;

    float delay_buf[DEAD_TIME_BUFFER] = {0};
    int delay_idx = 0;
    int delay_len = 0;

    absolute_time_t next_tick = make_timeout_time_ms(DEFAULT_DT_MS);
    LOGI("SIM core1 started, dt=%d ms\n", DEFAULT_DT_MS);

    while (true) {
        sim_config_t cfg;
        int reset_req = 0;

        critical_section_enter_blocking(&g_sim.lock);
        cfg = g_sim.cfg;
        if (g_sim.reset_requested) {
            reset_req = 1;
            g_sim.reset_requested = 0;
        }
        critical_section_exit(&g_sim.lock);

        if (reset_req) {
            LOGI("SIM reset requested\n");
            /* Reset PID state without enforcing output limits. */
            pid_init(&pid, cfg.pid.kp, cfg.pid.ki, cfg.pid.kd, 1.0f, -1.0f);
            second_state.state1 = 0.0f;
            second_state.state2 = 0.0f;
            y = 25.0f;
            u = 0.0f;
            delay_idx = 0;
            delay_len = 0;
        }

        int dt_ms = cfg.dt_ms;
        if (dt_ms < 1) dt_ms = 1;
        if (dt_ms > 1000) dt_ms = 1000;
        float dt = dt_ms / 1000.0f;

        pid.kp = cfg.pid.kp;
        pid.ki = cfg.pid.ki;
        pid.kd = cfg.pid.kd;

        float active_setpoint = cfg.use_master_setpoint ? cfg.master_setpoint : cfg.setpoint;
        float setpoint = cfg.running ? active_setpoint : 0.0f;
        if (cfg.running) {
            float feedback = cfg.allow_sens_signal ? y : 0.0f;
            float error = setpoint - feedback;
            u = pid_step(&pid, error, dt);
        } else {
            u = 0.0f;
        }
        /* Apply actuator direction and limits based on UI selection. */
        u1 = actuator_apply(u, cfg.act_inject, cfg.act_absorb, cfg.act_min, cfg.act_max);

        int desired_len = (dt_ms > 0) ? (cfg.plant.dead_time_ms / dt_ms) : 0;
        if (desired_len < 0) desired_len = 0;
        if (desired_len >= DEAD_TIME_BUFFER) desired_len = DEAD_TIME_BUFFER - 1;
        delay_len = desired_len;

        delay_buf[delay_idx] = u1;
        int read_idx = delay_idx - delay_len;
        if (read_idx < 0) read_idx += DEAD_TIME_BUFFER;
        float u_delayed = delay_buf[read_idx];
        delay_idx = (delay_idx + 1) % DEAD_TIME_BUFFER;

        if (cfg.plant.model == PLANT_FIRST_ORDER) {
            first_order_params_t p = {cfg.plant.gain, cfg.plant.tau};
            y = plant_first_order_step(y, u_delayed, &p, dt);
        } else {
            second_order_params_t p = {cfg.plant.wn, cfg.plant.zeta, cfg.plant.gain};
            y = plant_second_order_step(&second_state, u_delayed, &p, dt);
        }
        LOGD("SIM step: sp=%.2f u=%.3f u1=%.3f y=%.2f\n", setpoint, u, u1, y);

        critical_section_enter_blocking(&g_sim.lock);
        g_sim.rt.time_s += dt;
        g_sim.rt.setpoint = setpoint;
        g_sim.rt.control = u;
        g_sim.rt.actuator = u1;
        g_sim.rt.output = y;
        critical_section_exit(&g_sim.lock);

        sleep_until(next_tick);
        next_tick = delayed_by_ms(next_tick, dt_ms);
    }
}

/** Launch the core1 worker so core0 can handle Wi-Fi and UI. */
void sim_worker_start(void) {
    multicore_launch_core1(core1_main);
}
