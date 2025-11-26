#include "synth.h"
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define CLOCK_PIN 13
#define CV_PIN    14
#define SYS_CLK   125000000.0f
#define V_SUPPLY  3.3f
#define MV_PER_HZ 0.00095f 

#define CLOCK_DIV   150.0f 

// --- STATIC VARs ---
static uint clk_slice, clk_chan;
static uint cv_slice,  cv_chan;
static uint32_t cv_wrap_config = 0;

void synth_init(void) {
    // ----- CLOCK PWM -----
    gpio_set_function(CLOCK_PIN, GPIO_FUNC_PWM);
    clk_slice = pwm_gpio_to_slice_num(CLOCK_PIN);
    clk_chan  = pwm_gpio_to_channel(CLOCK_PIN);

    pwm_config clk_cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&clk_cfg, CLOCK_DIV);
    pwm_config_set_wrap(&clk_cfg, 60000);              // arbitrary starting value
    pwm_init(clk_slice, &clk_cfg, true);
    pwm_set_chan_level(clk_slice, clk_chan, 30000);    // 50% duty for clock

    // ----- CV PWM -----
    gpio_set_function(CV_PIN, GPIO_FUNC_PWM);
    cv_slice = pwm_gpio_to_slice_num(CV_PIN);
    cv_chan  = pwm_gpio_to_channel(CV_PIN);

    pwm_config cv_cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cv_cfg, 4.0f);
    pwm_config_set_wrap(&cv_cfg, 4095);                // ~12-bit CV
    pwm_init(cv_slice, &cv_cfg, true);

    cv_wrap_config = 4095;
}

void synth_set_frequency(float freq_hz) {
    // Clamp frequency
    if (freq_hz < 10.0f)   freq_hz = 10.0f;
    if (freq_hz > 5000.0f) freq_hz = 5000.0f;

    // ---------- CLOCK PWM: adjust WRAP only ----------
    // f = SYS_CLK / (CLOCK_DIV * (wrap + 1))
    float wrap_f = (SYS_CLK / (CLOCK_DIV * freq_hz)) - 1.0f;

    if (wrap_f < 1.0f)       wrap_f = 1.0f;
    if (wrap_f > 65535.0f)   wrap_f = 65535.0f;

    uint16_t wrap = (uint16_t)(wrap_f + 0.5f);         // round to nearest

    pwm_set_wrap(clk_slice, wrap);
    pwm_set_chan_level(clk_slice, clk_chan, (wrap + 1) / 2);   // 50% duty

    // ---------- CV PWM ----------
    float v_cv = freq_hz * MV_PER_HZ;
    if (v_cv < 0.0f)      v_cv = 0.0f;
    if (v_cv > V_SUPPLY)  v_cv = V_SUPPLY;

    float duty = v_cv / V_SUPPLY;
    pwm_set_chan_level(cv_slice, cv_chan,
                       (uint32_t)(cv_wrap_config * duty));
}


void synth_gate_off(void) {
    // later
}