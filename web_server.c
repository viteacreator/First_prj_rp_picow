#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/tcp.h"

#include "debug.h"
#include "sim_state.h"

#define HTTP_PORT 80

/** Decode URL query strings so messages show correctly. */
static void url_decode(char *dst, size_t dst_len, const char *src) {
    size_t di = 0;
    for (size_t si = 0; src[si] != '\0' && di + 1 < dst_len; si++) {
        if (src[si] == '%' && src[si + 1] && src[si + 2] &&
            isxdigit((unsigned char)src[si + 1]) &&
            isxdigit((unsigned char)src[si + 2])) {
            char hex[3] = { src[si + 1], src[si + 2], '\0' };
            dst[di++] = (char)strtoul(hex, NULL, 16);
            si += 2;
        } else if (src[si] == '+') {
            dst[di++] = ' ';
        } else {
            dst[di++] = src[si];
        }
    }
    dst[di] = '\0';
}

/** Extract a float query parameter from the URL. */
static int get_query_float(const char *path, const char *key, float *out) {
    const char *q = strchr(path, '?');
    if (!q) return 0;
    q++;
    size_t key_len = strlen(key);
    while (*q) {
        if (strncmp(q, key, key_len) == 0 && q[key_len] == '=') {
            *out = (float)atof(q + key_len + 1);
            return 1;
        }
        q = strchr(q, '&');
        if (!q) break;
        q++;
    }
    return 0;
}

/** Extract an int query parameter from the URL. */
static int get_query_int(const char *path, const char *key, int *out) {
    const char *q = strchr(path, '?');
    if (!q) return 0;
    q++;
    size_t key_len = strlen(key);
    while (*q) {
        if (strncmp(q, key, key_len) == 0 && q[key_len] == '=') {
            *out = atoi(q + key_len + 1);
            return 1;
        }
        q = strchr(q, '&');
        if (!q) break;
        q++;
    }
    return 0;
}

/** Apply configuration updates based on query parameters. */
static void apply_config_from_query(const char *path) {
    float value;
    int ivalue;

    const float min_gain = 0.0f;
    const float max_gain = 10.0f;
    const float min_tau = 0.1f;
    const float max_tau = 60.0f;
    const float min_wn = 0.1f;
    const float max_wn = 10.0f;
    const float min_zeta = 0.0f;
    const float max_zeta = 2.0f;
    const int min_dead = 0;
    const int max_dead = 2560;
    const float min_act = -1000.0f;
    const float max_act = 1000.0f;
    const int min_dt = 1;
    const int max_dt = 1000;

    critical_section_enter_blocking(&g_sim.lock);

    if (get_query_float(path, "setpoint", &value)) g_sim.cfg.setpoint = value;
    if (get_query_float(path, "kp", &value)) g_sim.cfg.pid.kp = value;
    if (get_query_float(path, "ki", &value)) g_sim.cfg.pid.ki = value;
    if (get_query_float(path, "kd", &value)) g_sim.cfg.pid.kd = value;
    if (get_query_int(path, "dt", &ivalue)) {
        if (ivalue < min_dt) ivalue = min_dt;
        if (ivalue > max_dt) ivalue = max_dt;
        g_sim.cfg.dt_ms = ivalue;
    }
    if (get_query_float(path, "gain", &value)) {
        if (value < min_gain) value = min_gain;
        if (value > max_gain) value = max_gain;
        g_sim.cfg.plant.gain = value;
    }
    if (get_query_float(path, "tau", &value)) {
        if (value < min_tau) value = min_tau;
        if (value > max_tau) value = max_tau;
        g_sim.cfg.plant.tau = value;
    }
    if (get_query_float(path, "wn", &value)) {
        if (value < min_wn) value = min_wn;
        if (value > max_wn) value = max_wn;
        g_sim.cfg.plant.wn = value;
    }
    if (get_query_float(path, "zeta", &value)) {
        if (value < min_zeta) value = min_zeta;
        if (value > max_zeta) value = max_zeta;
        g_sim.cfg.plant.zeta = value;
    }
    if (get_query_int(path, "dead", &ivalue)) {
        if (ivalue < min_dead) ivalue = min_dead;
        if (ivalue > max_dead) ivalue = max_dead;
        g_sim.cfg.plant.dead_time_ms = ivalue;
    }
    if (get_query_float(path, "act_min", &value)) {
        if (value < min_act) value = min_act;
        if (value > max_act) value = max_act;
        g_sim.cfg.act_min = value;
    }
    if (get_query_float(path, "act_max", &value)) {
        if (value < min_act) value = min_act;
        if (value > max_act) value = max_act;
        g_sim.cfg.act_max = value;
    }

    if (get_query_int(path, "model", &ivalue)) {
        g_sim.cfg.plant.model = (ivalue == 1) ? PLANT_SECOND_ORDER : PLANT_FIRST_ORDER;
    }
    if (get_query_int(path, "act_inject", &ivalue)) g_sim.cfg.act_inject = ivalue ? 1 : 0;
    if (get_query_int(path, "act_absorb", &ivalue)) g_sim.cfg.act_absorb = ivalue ? 1 : 0;
    if (get_query_int(path, "use_master", &ivalue)) g_sim.cfg.use_master_setpoint = ivalue ? 1 : 0;
    if (get_query_int(path, "allow_sens", &ivalue)) g_sim.cfg.allow_sens_signal = ivalue ? 1 : 0;
    if (get_query_int(path, "run", &ivalue)) g_sim.cfg.running = ivalue ? 1 : 0;
    if (get_query_int(path, "reset", &ivalue) && ivalue) g_sim.reset_requested = 1;

    if (g_sim.cfg.act_min > g_sim.cfg.act_max) {
        float tmp = g_sim.cfg.act_min;
        g_sim.cfg.act_min = g_sim.cfg.act_max;
        g_sim.cfg.act_max = tmp;
    }

    critical_section_exit(&g_sim.lock);
}

/** Build the JSON response for the current simulation state. */
static void build_state_json(char *out, size_t out_len) {
    sim_config_t cfg;
    sim_runtime_t rt;
    int reset_req;

    critical_section_enter_blocking(&g_sim.lock);
    cfg = g_sim.cfg;
    rt = g_sim.rt;
    reset_req = g_sim.reset_requested;
    critical_section_exit(&g_sim.lock);

    snprintf(out, out_len,
        "{"
        "\"running\":%d,"
        "\"setpoint\":%.2f,"
        "\"setpoint_cfg\":%.2f,"
        "\"master_setpoint\":%.2f,"
        "\"use_master\":%d,"
        "\"allow_sens\":%d,"
        "\"kp\":%.3f,"
        "\"ki\":%.3f,"
        "\"kd\":%.3f,"
        "\"dt\":%d,"
        "\"model\":%d,"
        "\"gain\":%.2f,"
        "\"tau\":%.2f,"
        "\"wn\":%.2f,"
        "\"zeta\":%.2f,"
        "\"dead\":%d,"
        "\"time\":%.2f,"
        "\"control\":%.3f,"
        "\"actuator\":%.3f,"
        "\"output\":%.2f,"
        "\"reset\":%d,"
        "\"act_inject\":%d,"
        "\"act_absorb\":%d,"
        "\"act_min\":%.2f,"
        "\"act_max\":%.2f"
        "}",
        cfg.running,
        rt.setpoint,
        cfg.setpoint,
        cfg.master_setpoint,
        cfg.use_master_setpoint,
        cfg.allow_sens_signal,
        cfg.pid.kp,
        cfg.pid.ki,
        cfg.pid.kd,
        cfg.dt_ms,
        (int)cfg.plant.model,
        cfg.plant.gain,
        cfg.plant.tau,
        cfg.plant.wn,
        cfg.plant.zeta,
        cfg.plant.dead_time_ms,
        rt.time_s,
        rt.control,
        rt.actuator,
        rt.output,
        reset_req,
        cfg.act_inject,
        cfg.act_absorb,
        cfg.act_min,
        cfg.act_max);
}

/** Build the HTML shell (JS is served separately at /app.js). */
static void build_page(char *out, size_t out_len) {
    snprintf(out, out_len,
        "<!doctype html>"
        "<html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Pico W PID Simulator</title>"
        "<style>"
        "body{font-family:Verdana,Arial,sans-serif;background:#f5f2e9;margin:20px;}"
        ".card{background:#fff;border:2px solid #222;padding:12px;margin-bottom:12px;}"
        ".controls{display:flex;flex-wrap:wrap;gap:8px;align-items:center;margin-top:8px;}"
        "button{padding:8px 12px;margin:4px;border:2px solid #222;background:#eae2d0;}"
        "button:active{background:#f5f2e9;}"
        "button.is-active{background:#f5f2e9;}"
        "input:not([type='checkbox']),select{padding:6px;border:2px solid #222;margin:4px;width:110px;}"
        "input[type='checkbox']{width:auto;padding:0;border:0;margin:0;vertical-align:middle;}"
        "input[type='color']{appearance:none;-webkit-appearance:none;padding:0;border:2px solid #222;margin:0;width:28px;height:20px;background:transparent;}"
        "canvas{border:2px solid #222;background:#fff;}"
        ".diag-block{fill:#fff;stroke:#222;stroke-width:2;}"
        ".diag-line{stroke:#222;stroke-width:2;fill:none;}"
        ".diag-text{font-size:12px;fill:#222;}"
        ".diag-title{font-size:12px;fill:#222;font-weight:bold;}"
        ".switch-active{fill:#f5f2e9;}"
        ".switch-disabled{fill:#ffbaba;}"
        ".master-border{fill:#fff;stroke:#777;}"
        ".master-box{fill:#f0f0f0;stroke:#777;}"
        ".master-text{fill:#777;}"
        ".schematic-shift{margin-left:140px;}"
        "</style></head><body>"
        "<h2>Pico W PID Simulator</h2>"
        "<div id='help_panel' style='display:none;border:2px solid #222;background:#fff;padding:12px;margin-bottom:12px;'>"
"<div style='font-weight:bold;margin-bottom:8px;'>On this page you can see a diagram of an automatic system in a closed-loop feedback.</div>"
"<div style='font-weight:bold;margin-top:10px;'>Blocks and signals</div>"
"<div style='margin-top:6px;'><b>Setpoint reg</b>: Here you set the value you want the system to achieve in the process.</div>"
"<div style='margin-top:4px;'><b>Summing junction (comparator)</b>: Does a simple operation to close the negative feedback loop: e(t) = r(t) - y1(t)</div>"
"<div style='margin-top:4px;'><b>PID controller</b>: The PID mathematical algorithm with e(t) as the input signal and u(t) as the result (controller output).</div>"
"<div style='margin-top:6px;'><b>Inputs you can set (tuning parameters)</b>:</div>"
"<div style='margin-left:14px;'>Kp - proportional gain (how strongly the controller reacts to the current error)</div>"
"<div style='margin-left:14px;'>Ki - integral gain (how strongly it reacts to accumulated error over time)</div>"
"<div style='margin-left:14px;'>Kd - derivative gain (how strongly it reacts to how fast the error is changing)</div>"
"<div style='margin-left:14px;'>dT (ms) - controller time step / sampling period used by the PID calculations</div>"
"<div style='margin-top:6px;'><b>Actuator / FCE</b>: As we simulate a real system, understand that an actuator can:</div>"
"<div style='margin-left:14px;'>Inject energy - actuator can only add energy to the system (e.g., heater in an oven)</div>"
"<div style='margin-left:14px;'>Absorb energy - actuator can only remove energy from the system (e.g., refrigerator)</div>"
"<div style='margin-left:14px;'>Or both, for example a motor drive which can accelerate and brake the motor.</div>"
"<div style='margin-top:4px;'>Also, a real actuator cannot inject or absorb infinite energy into/from the system. This is why you can set:</div>"
"<div style='margin-left:14px;'>Min / Max - clamps the actuator output to a allowed range (you can leave it as it is: -100...100, e.g. %% of power, PWM, Current, etc)</div>"
"<div style='margin-top:6px;'><b>Plant / Process</b>: This is the system being controlled (the \"process\"). More exactly, it is the mathematical equation that describes it.</div>"
"<div style='margin-top:4px;'>You can choose the mathematical model: First order or Second order, and set parameters:</div>"
"<div style='margin-left:14px;'>Gain (K) - overall plant gain</div>"
"<div style='margin-left:14px;'>Tau (tau) - time constant (first-order model)</div>"
"<div style='margin-left:14px;'>Wn (wn) - natural frequency (second-order model)</div>"
"<div style='margin-left:14px;'>Zeta (zeta) - damping ratio (second-order model)</div>"
"<div style='margin-left:14px;'>Dead (ms) - dead time / input delay before the plant responds</div>"
"<div style='margin-top:6px;'><b>Sensor</b>: Measures the plant output for feedback (often modeled as gain = 1).</div>"
"<div style='font-weight:bold;margin-top:10px;'>Controls (buttons)</div>"
"<div style='margin-left:14px;'>Apply - send/use the current settings (parameters) in the simulation</div>"
"<div style='margin-left:14px;'>Start - starts running the control loop (run setpoint + PID controller)</div>"
"<div style='margin-left:14px;'>Stop - stops the control loop (stop setpoint + PID controller only); rest of the system is still simulated, as in real life</div>"
"<div style='margin-left:14px;'>Reset - resets the PID internal state and disconnects the setpoint from r(t)</div>"
"<div style='margin-left:14px;'>Clear Plot - clears the plotted history (graph only)</div>"
"</div>"
        
        "<div class='card'><div><b>Status</b>: <span id='running'>STOPPED</span></div></div>"
        "<div class='card'>"
        "<svg id='control_diagram' width='1600' height='420' viewBox='0 0 1600 420'>"
        "<defs>"
        "<marker id='arrow' markerWidth='10' markerHeight='7' refX='10' refY='3.5' orient='auto'>"
        "<polygon points='0 0, 10 3.5, 0 7' fill='#222' />"
        "</marker>"
        "</defs>"
        "<rect id='sp_block' class='diag-block' x='20' y='145' width='140' height='75'/>"
        "<text class='diag-title' x='90' y='165' text-anchor='middle'>Setpoint reg</text>"
        "<foreignObject x='30' y='172' width='120' height='34'>"
        "<div xmlns='http://www.w3.org/1999/xhtml' style='display:flex;justify-content:center;'>"
        "<input id='setpoint' value='200' style='width:80px;'/>"
        "</div></foreignObject>"
        "<rect id='master_block' class='diag-block master-border' x='20' y='240' width='140' height='140'/>"
        "<text class='diag-title master-text' x='90' y='265' text-anchor='middle'>Master app control</text>"
        "<rect id='master_value_box' class='diag-block master-box' x='40' y='290' width='100' height='30'/>"
        "<text id='master_value' class='diag-text master-text' x='90' y='310' text-anchor='middle'>0</text>"
        "<polyline id='line_master_to_switch' class='diag-line' points='160,290 215,290 215,205' marker-end='url(#arrow)'/>"
        "<text class='diag-text' x='165' y='285'>m_r(t)</text>"
        "<polyline id='line_master_to_sw1_dash' class='diag-line' points='160,310 230,310 230,205' style='stroke:#777;stroke-dasharray:4 3;'/>"
        "<polyline id='line_master_to_sw2' class='diag-line' points='160,330 315,330 315,305' style='stroke:#777;stroke-dasharray:4 3;'/>"
        "<line id='line_r' class='diag-line' x1='160' y1='180' x2='200' y2='180' marker-end='url(#arrow)'/>"
        "<text class='diag-text' x='168' y='170'>r(t)</text>"
        "<rect id='pre_block' class='diag-block switch-disabled' x='200' y='155' width='50' height='50'/>"
        "<text class='diag-text' x='225' y='150' text-anchor='middle'>SW1</text>"
        "<line id='pre_block_line' class='diag-line' x1='200' y1='180' x2='250' y2='180'></line>"
        "<g id='diagram_shift' transform='translate(100 0)'>"
        "<line id='line_r2' class='diag-line' x1='150' y1='180' x2='202' y2='180' marker-end='url(#arrow)'/>"
        "<circle id='sum_block' class='diag-block' cx='230' cy='180' r='28'/>"
        "<text class='diag-text' x='222' y='172'>+</text>"
        "<text class='diag-text' x='222' y='202'>-</text>"
        "<line id='line_e' class='diag-line' x1='258' y1='180' x2='290' y2='180' marker-end='url(#arrow)'/>"
        "<text class='diag-text' x='266' y='170'>e(t)</text>"
        "<rect id='pid_block' class='diag-block' x='290' y='120' width='260' height='120'/>"
        "<text class='diag-title' x='420' y='148' text-anchor='middle'>PID controller</text>"
        "<foreignObject x='295' y='160' width='250' height='50'>"
        "<div xmlns='http://www.w3.org/1999/xhtml' style='display:flex;gap:0px;justify-content:center;align-items:flex-start;font-size:12px;'>"
        "<div style='display:flex;flex-direction:column;align-items:center;'><div>Kp</div><input id='kp' value='0.000' style='width:45px;padding:6px;margin:2px;'/></div>"
        "<div style='display:flex;flex-direction:column;align-items:center;'><div>Ki</div><input id='ki' value='0.000' style='width:45px;padding:6px;margin:2px;'/></div>"
        "<div style='display:flex;flex-direction:column;align-items:center;'><div>Kd</div><input id='kd' value='0.000' style='width:45px;padding:6px;margin:2px;'/></div>"
        "<div style='display:flex;flex-direction:column;align-items:center;'><div>dT (ms)</div><input id='dt' value='10' style='width:36px;padding:6px;margin:2px;'/></div>"
        "</div></foreignObject>"
        "<line id='line_u' class='diag-line' x1='550' y1='180' x2='600' y2='180' marker-end='url(#arrow)'/>"
        "<text class='diag-text' x='555' y='170'>u(t)</text>"
        "<rect id='act_block' class='diag-block' x='600' y='115' width='220' height='130'/>"
        "<text class='diag-title' x='705' y='133' text-anchor='middle'>Actuator / FCE</text>"
        "<foreignObject x='610' y='140' width='235' height='100'>"
        "<div xmlns='http://www.w3.org/1999/xhtml' style='font-size:12px;line-height:1.2;'>"
        "<label style='display:flex;align-items:center;gap:10px;margin:0 px;'>"
        "<input type='checkbox' id='act_inject' style='width:22px;height:22px;flex:0 0 22px;margin:0;padding:0;border:0;box-sizing:border-box;' onchange='saveSettings();updateActuatorModeUI()'>"
        "<span style='display:inline-block;line-height:22px;'>Inject energy</span>"
        "</label>"
        "<label style='display:flex;align-items:center;gap:8px;margin:6px 0 0 0;'>"
        "<input type='checkbox' id='act_absorb' style='width:22px;height:22px;flex:0 0 22px;margin:0;padding:0;border:0;box-sizing:border-box;' onchange='saveSettings();updateActuatorModeUI()'>"
        "<span style='display:inline-block;line-height:22px;'>Absorb energy</span>"
        "</label>"
        "<div style='margin-top:8px;display:flex;gap:6px;align-items:center;'>"
        "<span>Min</span>"
        "<input id='act_min' value='-100' style='width:50px;padding:6px;margin:2px;'/>"
        "<span>Max</span>"
        "<input id='act_max' value='100' style='width:50px;padding:6px;margin:2px;'/>"
        "</div>"
        "</div></foreignObject>"
        "<line id='line_u1' class='diag-line' x1='820' y1='180' x2='870' y2='180' marker-end='url(#arrow)'/>"
        "<text class='diag-text' x='825' y='170'>u1(t)</text>"
        "<rect id='plant_block' class='diag-block' x='870' y='110' width='360' height='215'/>"
        "<text class='diag-title' x='1050' y='128' text-anchor='middle'>Plant / Process</text>"
        "<foreignObject x='885' y='134' width='330' height='180'>"
        "<div xmlns='http://www.w3.org/1999/xhtml' style='font-size:12px;'>"
        "<div style='display:flex;align-items:center;gap:8px;margin-bottom:4px;'><div style='font-weight:bold;'>Model</div><select id='model' style='width:240px;' onchange='updateModelUI();saveSettings()'><option value='0'>First order</option><option value='1' selected>Second order</option></select></div>"
        "<div style='display:flex;gap:10px;flex-wrap:wrap;'>"
        "<div id='row_gain' style='display:inline-flex;align-items:center;gap:6px;'>Gain <input id='gain' value='2.0' style='width:50px;padding:6px;margin:2px;'/></div>"
        "<div id='row_tau' style='display:inline-flex;align-items:center;gap:6px;'>Tau <input id='tau' value='8.0' style='width:50px;padding:6px;margin:2px;'/></div>"
        "<div id='row_wn' style='display:inline-flex;align-items:center;gap:6px;'>Wn <input id='wn' value='1.2' style='width:50px;padding:6px;margin:2px;'/></div>"
        "<div id='row_zeta' style='display:inline-flex;align-items:center;gap:6px;'>Zeta <input id='zeta' value='0.7' style='width:50px;padding:6px;margin:2px;'/></div>"
        "<div id='row_dead' style='display:inline-flex;align-items:center;gap:6px;'>Dead <input id='dead' value='0' style='width:50px;padding:6px;margin:2px;'/> ms</div>"
        "</div>"
        "<div style='border:2px solid #222;padding: 6px;margin-top: 6px;font-size: 12px;'><div id='tf_text'>Y(s)/U(s) = K*wn^2/(s^2+2*zeta*wn*s+wn^2) | K=5.00, wn=1.20, zeta=0.70</div></div>"
        "</div></foreignObject>"
        "<line id='line_y' class='diag-line' x1='1230' y1='180' x2='1300' y2='180' marker-end='url(#arrow)'/>"
        "<text class='diag-text' x='1300' y='175'>y(t)</text>"
        "<rect id='dist_block' class='diag-block' x='600' y='20' width='220' height='70'/>"
        "<text class='diag-title' x='700' y='45' text-anchor='middle'>External disturbance</text>"
        "<line id='line_pd1' class='diag-line' x1='820' y1='55' x2='1060' y2='55'/>"
        "<line id='line_pd2' class='diag-line' x1='1060' y1='55' x2='1060' y2='110' marker-end='url(#arrow)'/>"
        "<text class='diag-text' x='1070' y='80'>p(t)</text>"
        "<rect id='sens_block' class='diag-block' x='600' y='320' width='220' height='70'/>"
        "<text class='diag-title' x='705' y='342' text-anchor='middle'>Sensor</text>"
        "<rect class='diag-block' x='698' y='352' width='22' height='22'/>"
        "<text class='diag-text' x='709' y='368' text-anchor='middle'>1</text>"
        "<line id='fb_down' class='diag-line' x1='1260' y1='180' x2='1260' y2='355'/>"
        "<line id='fb_into_sensor' class='diag-line' x1='1260' y1='355' x2='820' y2='355' marker-end='url(#arrow)'/>"
        "<line id='fb_out_sensor' class='diag-line' x1='600' y1='355' x2='230' y2='355'/>"
        "<rect id='fb_switch' class='diag-block' x='205' y='260' width='50' height='50'/>"
        "<text class='diag-text' x='275' y='290' text-anchor='middle'>SW2</text>"
        "<line id='fb_switch_line' class='diag-line' x1='230' y1='260' x2='230' y2='310'/>"
        "<text id='fb_switch_text' class='diag-text' x='230' y='286' text-anchor='middle' style='display:none;'>0</text>"
        "<line id='fb_up_to_sw2' class='diag-line' x1='230' y1='260' x2='230' y2='208' marker-end='url(#arrow)'/>"
        "<text class='diag-text' x='246' y='228'>y1(t)</text>"
        "<line id='fb_up_to_sum' class='diag-line' x1='230' y1='355' x2='230' y2='310'/>"
        "</g>"
        "</svg>"
        "<div class='controls'>"
        "<div>Time window (s) <input id='window' type='number' min='10' step='10' value='100' onchange='setWindow();saveSettings()'></div>"
        "<div>Plot W <input id='plot_w' type='number' min='300' step='10' value='1400' onchange='setPlotSize();saveSettings()'></div>"
        "<div>Plot H <input id='plot_h' type='number' min='200' step='10' value='700' onchange='setPlotSize();saveSettings()'></div>"
        "<button onclick='sendConfig()'>Apply</button>"
        "<button id='btn_start' onclick='startSim()'>Start</button>"
        "<button id='btn_stop' onclick='stopSim()'>Stop</button>"
        "<button onclick='resetSim()'>Reset</button>"
        "<button onclick='clearPlot()'>Clear Plot</button>"
        "<button id='btn_help' onclick='toggleHelp()'>Help</button>"
        "</div>"
        "</div>"
        "<div class='card'>"
        "<div style='display:flex;gap:16px;align-items:center;flex-wrap:wrap;'>"
        "<div><b>Time t</b>: <span id='time'>0</span> s</div>"
        "<div><b>Output y(t)</b>: <span id='output'>0</span></div>"
        "<div><b>Control u(t)</b>: <span id='control'>0</span></div>"
        "<div><b>Actuator u1(t)</b>: <span id='actuator'>0</span></div>"
        "</div>"
        "</div>"
        "<div class='card'>"
        "<div style='display:flex;gap:20px;align-items:center;flex-wrap:wrap;justify-content:flex-start;'>"
        "<div style='display:inline-flex;gap:8px;align-items:center;'>"
        "<label style='display:inline-flex;gap:6px;align-items:center;margin:0;'><input type='checkbox' id='show_sp' checked onchange='saveSettings();draw()' style='margin:0;'>Setpoint</label>"
        "<input type='color' id='color_sp' value='#000000' onchange='saveSettings();updateColorPickers();draw()' style='margin:0;width:28px;height:20px;'>"
        "</div>"
        "<div style='display:inline-flex;gap:8px;align-items:center;'>"
        "<label style='display:inline-flex;gap:6px;align-items:center;margin:0;'><input type='checkbox' id='show_u' checked onchange='saveSettings();draw()' style='margin:0;'>Control u</label>"
        "<input type='color' id='color_u' value='#9b9b9b' onchange='saveSettings();updateColorPickers();draw()' style='margin:0;width:28px;height:20px;'>"
        "</div>"
        "<div style='display:inline-flex;gap:8px;align-items:center;'>"
        "<label style='display:inline-flex;gap:6px;align-items:center;margin:0;'><input type='checkbox' id='show_u1' checked onchange='saveSettings();draw()' style='margin:0;'>Actuator u1</label>"
        "<input type='color' id='color_u1' value='#6abf4b' onchange='saveSettings();updateColorPickers();draw()' style='margin:0;width:28px;height:20px;'>"
        "</div>"
        "<div style='display:inline-flex;gap:8px;align-items:center;'>"
        "<label style='display:inline-flex;gap:6px;align-items:center;margin:0;'><input type='checkbox' id='show_y' checked onchange='saveSettings();draw()' style='margin:0;'>Output y</label>"
        "<input type='color' id='color_y' value='#b00000' onchange='saveSettings();updateColorPickers();draw()' style='margin:0;width:28px;height:20px;'>"
        "</div>"
        "</div>"
        "<canvas id='plot' width='1400' height='700'></canvas>"
        "</div>"
        "<script src='/app.js'></script>"
        "</body></html>");
}/** Build the JavaScript application served at /app.js. */
static void build_app_js(char *out, size_t out_len) {
    snprintf(out, out_len,
        "/* Sampling and history buffers for plotting. */"
        "var sampleMs=200;var windowSec=100;var hist=500;var sp=[],y=[],u=[],u1=[];var synced=false;var tfPending=false;var runPending=false;var currentTime=0;var useMasterSetpoint=false;var allowSensSignal=true;"
        "function q(id){return document.getElementById(id)}"
        "function api(url,cb){"
        "var x=new XMLHttpRequest();"
        "x.onreadystatechange=function(){"
        "if(x.readyState===4&&x.status===200){"
        "try{cb(JSON.parse(x.responseText));}catch(e){}}};"
        "x.open('GET',url,true);x.setRequestHeader('Cache-Control','no-cache');x.send();"
        "}"
        "/* Persist UI settings so a refresh keeps the same values. */"
        "function saveSettings(){"
        "var s={"
        "setpoint:q('setpoint').value,"
        "kp:q('kp').value,ki:q('ki').value,kd:q('kd').value,dt:q('dt').value,"
        "model:q('model').value,gain:q('gain').value,tau:q('tau').value,"
        "wn:q('wn').value,zeta:q('zeta').value,dead:q('dead').value,"
        "act_min:q('act_min').value,act_max:q('act_max').value,"
        "window:q('window').value,plot_w:q('plot_w').value,plot_h:q('plot_h').value,"
        "show_sp:q('show_sp').checked,show_u:q('show_u').checked,show_u1:q('show_u1').checked,show_y:q('show_y').checked,"
        "color_sp:q('color_sp').value,color_u:q('color_u').value,color_u1:q('color_u1').value,color_y:q('color_y').value,"
        "act_inject:q('act_inject').checked,act_absorb:q('act_absorb').checked"
        "};"
        "try{localStorage.setItem('pid_settings',JSON.stringify(s));}catch(e){}"
        "}"
        "/* Restore UI settings if they were saved previously. */"
        "function loadSettings(){"
        "try{var raw=localStorage.getItem('pid_settings');if(!raw)return false;"
        "var s=JSON.parse(raw);"
        "if(s.setpoint!==undefined)q('setpoint').value=s.setpoint;"
        "if(s.kp!==undefined)q('kp').value=s.kp;"
        "if(s.ki!==undefined)q('ki').value=s.ki;"
        "if(s.kd!==undefined)q('kd').value=s.kd;"
        "if(s.dt!==undefined)q('dt').value=s.dt;"
        "if(s.model!==undefined)q('model').value=s.model;"
        "if(s.gain!==undefined)q('gain').value=s.gain;"
        "if(s.tau!==undefined)q('tau').value=s.tau;"
        "if(s.wn!==undefined)q('wn').value=s.wn;"
        "if(s.zeta!==undefined)q('zeta').value=s.zeta;"
        "if(s.dead!==undefined)q('dead').value=s.dead;"
        "if(s.act_min!==undefined)q('act_min').value=s.act_min;"
        "if(s.act_max!==undefined)q('act_max').value=s.act_max;"
        "if(s.window!==undefined)q('window').value=s.window;"
        "if(s.plot_w!==undefined)q('plot_w').value=s.plot_w;"
        "if(s.plot_h!==undefined)q('plot_h').value=s.plot_h;"
        "if(s.show_sp!==undefined)q('show_sp').checked=!!s.show_sp;"
        "if(s.show_u!==undefined)q('show_u').checked=!!s.show_u;"
        "if(s.show_u1!==undefined)q('show_u1').checked=!!s.show_u1;"
        "if(s.show_y!==undefined)q('show_y').checked=!!s.show_y;"
        "if(s.color_sp!==undefined)q('color_sp').value=s.color_sp;"
        "if(s.color_u!==undefined)q('color_u').value=s.color_u;"
        "if(s.color_u1!==undefined)q('color_u1').value=s.color_u1;"
        "if(s.color_y!==undefined)q('color_y').value=s.color_y;"
        "if(s.act_inject!==undefined)q('act_inject').checked=!!s.act_inject;"
        "if(s.act_absorb!==undefined)q('act_absorb').checked=!!s.act_absorb;"
        "updateColorPickers();"
        "updateActuatorModeUI();"
        "updateModelUI();"
        "setWindow();setPlotSize();"
        "return true;}catch(e){return false;}"
        "}"
        "/* Send configuration changes to the device. */"
        "function sendConfig(){"
        "var url='/api/set?setpoint='+encodeURIComponent(q('setpoint').value)"
        "+'&kp='+encodeURIComponent(q('kp').value)"
        "+'&ki='+encodeURIComponent(q('ki').value)"
        "+'&kd='+encodeURIComponent(q('kd').value)"
        "+'&dt='+encodeURIComponent(q('dt').value)"
        "+'&model='+encodeURIComponent(q('model').value)"
        "+'&gain='+encodeURIComponent(q('gain').value)"
        "+'&tau='+encodeURIComponent(q('tau').value)"
        "+'&wn='+encodeURIComponent(q('wn').value)"
        "+'&zeta='+encodeURIComponent(q('zeta').value)"
        "+'&dead='+encodeURIComponent(q('dead').value)"
        "+'&act_min='+encodeURIComponent(q('act_min').value)"
        "+'&act_max='+encodeURIComponent(q('act_max').value)"
        "+'&act_inject='+(q('act_inject').checked?1:0)"
        "+'&act_absorb='+(q('act_absorb').checked?1:0);"
        "saveSettings();tfPending=true;runPending=true;api(url,function(d){"
        "if(d&&tfPending){q('tf_text').textContent=transferText(d);tfPending=false;}"
        "updateUI(d);});}"
        "/* Recalculate history length when time window changes. */"
        "function setWindow(){"
        "var v=parseFloat(q('window').value);"
        "if(isNaN(v)||v<2)v=2;"
        "windowSec=v;"
        "hist=Math.floor((windowSec*1000)/sampleMs)+1;"
        "if(hist<10)hist=10;"
        "while(sp.length>hist){sp.shift();y.shift();u.shift();u1.shift();}"
        "}"
        "function setPlotSize(){"
        "var c=q('plot');"
        "var w=parseInt(q('plot_w').value,10);"
        "var h=parseInt(q('plot_h').value,10);"
        "if(isNaN(w)||w<300)w=300;"
        "if(isNaN(h)||h<200)h=200;"
        "c.width=w;c.height=h;draw();"
        "}"
        "function startSim(){setRunButtonsState(true);runPending=true;api('/api/set?run=1',updateUI)}"
        "function stopSim(){setRunButtonsState(false);runPending=true;api('/api/set?run=0',updateUI)}"
        "function resetSim(){runPending=true;api('/api/set?reset=1',updateUI)}"
        "/* Clear plotted history without changing configuration. */"
        "function clearPlot(){sp=[];y=[];u=[];u1=[];draw();}"
        "function toggleHelp(){"
        "var p=q('help_panel');"
        "var open=(p.style.display==='none'||p.style.display==='');"
        "p.style.display=open?'block':'none';"
        "setHelpButtonState(open);"
        "}"
        "function setHelpButtonState(open){"
        "var b=q('btn_help');"
        "if(!b)return;"
        "if(open)b.classList.add('is-active');else b.classList.remove('is-active');"
        "}"
        "function setRunButtonsState(running){"
        "var bs=q('btn_start');var bp=q('btn_stop');"
        "if(bs){if(running)bs.classList.add('is-active');else bs.classList.remove('is-active');}"
        "if(bp){if(running)bp.classList.remove('is-active');else bp.classList.add('is-active');}"
        "}"
        "function setSwitchLine(useMaster){"
        "var line=q('pre_block_line');var block=q('pre_block');"
        "if(!line||!block)return;"
        "if(useMaster){"
        "line.setAttribute('x1','215');line.setAttribute('y1','205');line.setAttribute('x2','250');line.setAttribute('y2','180');"
        "block.classList.add('switch-disabled');block.classList.remove('switch-active');"
        "}else{"
        "line.setAttribute('x1','200');line.setAttribute('y1','180');line.setAttribute('x2','250');line.setAttribute('y2','180');"
        "block.classList.remove('switch-disabled');block.classList.add('switch-active');"
        "}"
        "useMasterSetpoint=useMaster;"
        "}"
        "function toggleSetpointSource(){"
        "var next=!useMasterSetpoint;"
        "setSwitchLine(next);"
        "api('/api/set?use_master='+(next?1:0),updateUI);"
        "}"
        "function setFeedbackSwitch(allow){"
        "var line=q('fb_switch_line');var text=q('fb_switch_text');var block=q('fb_switch');"
        "if(!line||!text||!block)return;"
        "if(allow){"
        "line.style.display='';text.style.display='none';block.classList.remove('switch-disabled');block.classList.add('switch-active');"
        "}else{"
        "line.style.display='none';text.style.display='';block.classList.remove('switch-active');block.classList.add('switch-disabled');"
        "}"
        "allowSensSignal=allow;"
        "}"
        "function toggleFeedbackSwitch(){"
        "var next=!allowSensSignal;"
        "setFeedbackSwitch(next);"
        "api('/api/set?allow_sens='+(next?1:0),updateUI);"
        "}"
        "/* Sync incoming state into UI fields and plotting buffers. */"
        "function updateUI(d){"
        "if(!d)return;"
        "if(runPending){q('running').textContent=d.running?'RUNNING':'STOPPED';runPending=false;}"
        "setRunButtonsState(!!d.running);"
        "updateModelUI();"
        "currentTime=d.time||0;"
        "q('time').textContent=d.time.toFixed(2);"
        "q('output').textContent=d.output.toFixed(2);"
        "q('control').textContent=d.control.toFixed(3);"
        "q('actuator').textContent=d.actuator.toFixed(3);"
        "if(q('master_value'))q('master_value').textContent=d.master_setpoint.toFixed(2);"
        "useMasterSetpoint=!!d.use_master;"
        "allowSensSignal=!!d.allow_sens;"
        "if(!synced){"
        "if(!loadSettings()){"
        "q('setpoint').value=d.setpoint_cfg.toFixed(2);"
        "q('kp').value=d.kp.toFixed(3);"
        "q('ki').value=d.ki.toFixed(3);"
        "q('kd').value=d.kd.toFixed(3);"
        "q('dt').value=d.dt;"
        "q('model').value=d.model.toString();"
        "q('gain').value=d.gain.toFixed(2);"
        "q('tau').value=d.tau.toFixed(2);"
        "q('wn').value=d.wn.toFixed(2);"
        "q('zeta').value=d.zeta.toFixed(2);"
        "q('dead').value=d.dead;"
        "q('act_min').value=d.act_min.toFixed(2);"
        "q('act_max').value=d.act_max.toFixed(2);"
        "q('act_inject').checked=!!d.act_inject;"
        "q('act_absorb').checked=!!d.act_absorb;"
        "setWindow();setPlotSize();"
        "}"
        "synced=true;"
        "}"
        "sp.push(d.setpoint);y.push(d.output);u.push(d.control);u1.push(d.actuator);"
        "if(sp.length>hist){sp.shift();y.shift();u.shift();u1.shift();}"
        "updateActuatorModeUI();"
        "setSwitchLine(useMasterSetpoint);"
        "setFeedbackSwitch(allowSensSignal);"
        "draw();}"
        "/* Build a readable transfer function string. */"
        "function transferText(d){"
        "if(d.model===0){"
        "var eq='Y(s)/U(s) = K/(tau*s+1)';"
        "eq+=' | K='+d.gain.toFixed(2)+', tau='+d.tau.toFixed(2);"
        "return eq;"
        "}"
        "var eq='Y(s)/U(s) = K*wn^2/(s^2+2*zeta*wn*s+wn^2)';"
        "eq+=' | K='+d.gain.toFixed(2)+', wn='+d.wn.toFixed(2)+', zeta='+d.zeta.toFixed(2);"
        "return eq;"
        "}"
        "/* Show only the parameters relevant to the selected model. */"
        "function updateModelUI(){"
        "var firstOrder=(q('model').value==='0');"
        "q('row_tau').style.display=firstOrder?'inline-flex':'none';"
        "q('row_wn').style.display=firstOrder?'none':'inline-flex';"
        "q('row_zeta').style.display=firstOrder?'none':'inline-flex';"
        "}"
        "/* Highlight actuator block based on inject/absorb selection. */"
        "function updateActuatorModeUI(){"
        "var inject=q('act_inject').checked;"
        "var absorb=q('act_absorb').checked;"
        "var block=q('act_block');"
        "if(!block)return;"
        "var fill='#f3ffeeff';"
        "if(inject&&!absorb)fill='#fffce4ff';"
        "else if(!inject&&absorb)fill='#f0f8ffff';"
        "else if(!inject&&!absorb)fill='#ffbabaff';"
        "block.style.fill=fill;"
        "}"
        "/* Keep color pickers filled with their selected color. */"
        "function updateColorPickers(){"
        "q('color_sp').style.background=q('color_sp').value;"
        "q('color_u').style.background=q('color_u').value;"
        "q('color_u1').style.background=q('color_u1').value;"
        "q('color_y').style.background=q('color_y').value;"
        "}"
        "/* Redraw the plot using the current buffers and toggles. */"
        "function draw(){"
        "var c=q('plot');var ctx=c.getContext('2d');"
        "var padL=36,padR=8,padT=8,padB=22;"
        "var w=c.width-padL-padR;var h=c.height-padT-padB;"
        "var inset=0.10*h;"
        "ctx.clearRect(0,0,c.width,c.height);"
        "ctx.strokeStyle='#222';ctx.strokeRect(padL,padT,w,h);"
        "var series=[];"
        "if(q('show_sp').checked)series.push(sp);"
        "if(q('show_u').checked)series.push(u);"
        "if(q('show_u1').checked)series.push(u1);"
        "if(q('show_y').checked)series.push(y);"
        "var vals=[];for(var si=0;si<series.length;si++){vals=vals.concat(series[si]);}"
        "if(vals.length===0){vals=[0,1];}"
        "var max=Math.max.apply(null,vals);"
        "var min=Math.min.apply(null,vals);"
        "var range=(max-min)||1;"
        "ctx.fillStyle='#222';ctx.font='10px Arial';"
        "ctx.fillText('Temp',2,padT+10);"
        "for(var i=0;i<=4;i++){"
        "var ty=padT+inset+(1-(i/4))*(h-2*inset);"
        "var val=(min+(i/4)*range).toFixed(0);"
        "ctx.fillText(val,2,ty+3);"
        "ctx.strokeStyle='#edededff';ctx.beginPath();ctx.moveTo(padL,ty);ctx.lineTo(padL+w,ty);ctx.stroke();"
        "}"
        "var t0=(currentTime-windowSec);if(t0<0)t0=0;"
        "ctx.fillStyle='#222';"
        "ctx.fillText('t0='+t0.toFixed(1)+'s',padL,padT+h+14);"
        "ctx.fillText('t='+currentTime.toFixed(1)+'s',padL+w-46,padT+h+14);"
        "function plot(arr,color){ctx.strokeStyle=color;ctx.beginPath();"
        "for(var i=0;i<arr.length;i++){var x=padL+i*(w/(hist-1));"
        "var ypix=padT+inset+(1-((arr[i]-min)/range))*(h-2*inset);"
        "if(i===0)ctx.moveTo(x,ypix);else ctx.lineTo(x,ypix);}ctx.stroke();}"
        "if(q('show_sp').checked)plot(sp,q('color_sp').value||'#000');"
        "if(q('show_u').checked)plot(u,q('color_u').value||'#9b9b9b');"
        "if(q('show_u1').checked)plot(u1,q('color_u1').value||'#6abf4b');"
        "if(q('show_y').checked)plot(y,q('color_y').value||'#b00');}"
        "setWindow();setPlotSize();"
        "updateModelUI();"
        "setSwitchLine(useMasterSetpoint);setFeedbackSwitch(allowSensSignal);"
        "var switchBlock=q('pre_block');if(switchBlock){switchBlock.addEventListener('click',toggleSetpointSource);}"
        "var feedbackSwitch=q('fb_switch');if(feedbackSwitch){feedbackSwitch.addEventListener('click',toggleFeedbackSwitch);}"
        "api('/api/state',updateUI);"
        "setInterval(function(){api('/api/state?t='+(new Date().getTime()),updateUI);},200);");
}

typedef struct {
    struct tcp_pcb *pcb;
    size_t header_len;
    size_t body_len;
    size_t offset;
    int active;
    char header[256];
    char body[16384];
} http_response_t;

static http_response_t g_resp;

/** Send as much as possible; continue via tcp_sent when more buffer is available. */
static int http_send_more(http_response_t *r) {
    while (r->offset < r->header_len + r->body_len) {
        u16_t snd = tcp_sndbuf(r->pcb);
        if (snd == 0) {
            return 0;
        }
        size_t to_write = r->header_len + r->body_len - r->offset;
        if (to_write > snd) to_write = snd;
        if (to_write > 256) to_write = 256;

        const char *src;
        size_t src_off;
        if (r->offset < r->header_len) {
            src = r->header;
            src_off = r->offset;
        } else {
            src = r->body;
            src_off = r->offset - r->header_len;
        }
        size_t avail = (src == r->header) ? (r->header_len - src_off) : (r->body_len - src_off);
        if (to_write > avail) to_write = avail;

        err_t err = tcp_write(r->pcb, src + src_off, to_write, TCP_WRITE_FLAG_COPY);
        if (err != ERR_OK) {
            LOGW("tcp_write failed: %d (offset=%u)\n", err, (unsigned)r->offset);
            return 0;
        }
        r->offset += to_write;
    }
    return 1;
}

static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    (void)len;
    http_response_t *r = (http_response_t *)arg;
    if (!r || r->pcb != tpcb) {
        return ERR_OK;
    }
    if (http_send_more(r)) {
        tcp_output(tpcb);
    }
    if (r->offset >= r->header_len + r->body_len) {
        tcp_close(tpcb);
        r->active = 0;
    }
    return ERR_OK;
}

static void http_err(void *arg, err_t err) {
    (void)err;
    http_response_t *r = (http_response_t *)arg;
    if (r) {
        r->active = 0;
    }
}

/** Send a small 503 response if the server is busy. */
static void http_send_busy(struct tcp_pcb *tpcb) {
    const char *msg =
        "HTTP/1.1 503 Busy\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "Connection: close\r\n\r\n"
        "Server busy";
    tcp_write(tpcb, msg, strlen(msg), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    tcp_close(tpcb);
}

/** Handle an incoming TCP packet and return the HTML or JSON response. */
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    (void)arg;
    (void)err;
    if (!p) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    static char req[512];

    size_t len = pbuf_copy_partial(p, req, sizeof(req) - 1, 0);
    tcp_recved(tpcb, p->tot_len);
    req[len] = '\0';
    pbuf_free(p);

    const char *path = "/";
    if (strncmp(req, "GET ", 4) == 0) {
        char *start = req + 4;
        char *space = strchr(start, ' ');
        if (space) {
            *space = '\0';
            path = start;
        }
    }

    LOGI("HTTP GET %s\n", path);
    if (strncmp(path, "/api/", 5) == 0) {
        LOGD("HTTP API request\n");
    }

    const char *content_type = "text/html";

    if (g_resp.active) {
        LOGW("HTTP busy, rejecting request\n");
        http_send_busy(tpcb);
        return ERR_OK;
    }

    if (strncmp(path, "/api/set", 8) == 0) {
        apply_config_from_query(path);
        build_state_json(g_resp.body, sizeof(g_resp.body));
        content_type = "application/json";
    } else if (strncmp(path, "/api/state", 10) == 0) {
        build_state_json(g_resp.body, sizeof(g_resp.body));
        content_type = "application/json";
    } else if (strncmp(path, "/app.js", 7) == 0) {
        build_app_js(g_resp.body, sizeof(g_resp.body));
        content_type = "application/javascript";
    } else {
        build_page(g_resp.body, sizeof(g_resp.body));
    }

    g_resp.body_len = strlen(g_resp.body);
    if (g_resp.body_len >= sizeof(g_resp.body) - 1) {
        LOGW("HTTP response truncated: %d bytes\n", (int)g_resp.body_len);
    }
    g_resp.header_len = (size_t)snprintf(g_resp.header, sizeof(g_resp.header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n\r\n",
             content_type, (int)g_resp.body_len);

    g_resp.pcb = tpcb;
    g_resp.offset = 0;
    g_resp.active = 1;
    tcp_arg(tpcb, &g_resp);
    tcp_sent(tpcb, http_sent);
    tcp_err(tpcb, http_err);

    if (http_send_more(&g_resp)) {
        tcp_output(tpcb);
    } else {
        LOGW("HTTP send pending (sndbuf empty)\n");
    }
    return ERR_OK;
}

/** Accept a new TCP connection and install the receive callback. */
static err_t http_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    (void)arg;
    (void)err;
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

/** Create the TCP listener for the web UI on port 80. */
bool start_http_server(void) {
    cyw43_arch_lwip_begin();
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        cyw43_arch_lwip_end();
        return false;
    }
    if (tcp_bind(pcb, NULL, HTTP_PORT) != ERR_OK) {
        tcp_close(pcb);
        cyw43_arch_lwip_end();
        return false;
    }
    struct tcp_pcb *listen_pcb = tcp_listen_with_backlog(pcb, 4);
    if (!listen_pcb) {
        tcp_close(pcb);
        cyw43_arch_lwip_end();
        return false;
    }
    pcb = listen_pcb;
    tcp_accept(pcb, http_accept);
    cyw43_arch_lwip_end();
    return true;
}





