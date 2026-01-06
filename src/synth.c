#include "synth.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// ---------------- Pins ----------------
#define CLOCK_PIN 13
#define CV_PIN    14
#define GATE_PIN  12

// ---------------- Clocks/rails ----------------
#define SYS_CLK_HZ 125000000.0f
#define V_SUPPLY   3.3f
#define V_SUPPLY_INV (1.0f / V_SUPPLY)

// ---------------- Clocked-reset DCO model ----------------
#define CLOCK_DIV     150.0f
#define CLOCK_DIV_INV (1.0f / CLOCK_DIV)
#define RESET_US      9.0f

// Pre-compute tick rate and reset pulse width
#define TICK_HZ       (SYS_CLK_HZ * CLOCK_DIV_INV)
#define RESET_TICKS   ((uint32_t)(TICK_HZ * RESET_US * 1e-6f + 0.5f))

// CV mapping
#define MV_PER_HZ     0.000060f
#define CV_GAIN       10.0f
#define CV_OFFSET     0.0f

// CV PWM carrier
#define CV_PWM_CARRIER_HZ 500000u

// Frequency limits
#define FREQ_MIN      10.0f
#define FREQ_MAX      20000.0f

// ---------------- Static state ----------------
static uint clk_slice, clk_chan;
static uint cv_slice,  cv_chan;
static uint32_t cv_wrap = 0;

void synth_init(void) {
    // ----- RESET CLOCK PWM -----
    gpio_set_function(CLOCK_PIN, GPIO_FUNC_PWM);
    clk_slice = pwm_gpio_to_slice_num(CLOCK_PIN);
    clk_chan  = pwm_gpio_to_channel(CLOCK_PIN);

    pwm_config clk_cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&clk_cfg, CLOCK_DIV);
    pwm_config_set_wrap(&clk_cfg, 60000);
    pwm_init(clk_slice, &clk_cfg, true);
    pwm_set_chan_level(clk_slice, clk_chan, 1);

    // ----- CV PWM -----
    gpio_set_function(CV_PIN, GPIO_FUNC_PWM);
    cv_slice = pwm_gpio_to_slice_num(CV_PIN);
    cv_chan  = pwm_gpio_to_channel(CV_PIN);

    // Compute wrap for carrier frequency
    cv_wrap = (uint32_t)(SYS_CLK_HZ / CV_PWM_CARRIER_HZ) - 1;

    pwm_config cv_cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cv_cfg, 1.0f);
    pwm_config_set_wrap(&cv_cfg, cv_wrap);
    pwm_init(cv_slice, &cv_cfg, true);
    pwm_set_chan_level(cv_slice, cv_chan, 0);

    // ----- GATE -----
    gpio_init(GATE_PIN);
    gpio_set_dir(GATE_PIN, GPIO_OUT);
    gpio_put(GATE_PIN, 0);
}

void synth_set_frequency(float freq_hz) {
    // Clamp frequency
    if (freq_hz < FREQ_MIN) freq_hz = FREQ_MIN;
    else if (freq_hz > FREQ_MAX) freq_hz = FREQ_MAX;

    // ---------- RESET CLOCK ----------
    float wrap_f = (SYS_CLK_HZ * CLOCK_DIV_INV / freq_hz) - 1.0f;
    
    // Clamp wrap value
    if (wrap_f < 1.0f) wrap_f = 1.0f;
    else if (wrap_f > 65535.0f) wrap_f = 65535.0f;
    
    uint16_t wrap = (uint16_t)(wrap_f + 0.5f);
    pwm_set_wrap(clk_slice, wrap);

    // ---------- RESET PULSE ----------
    // Using pre-computed constant, clamp to wrap
    uint32_t pulse = (RESET_TICKS > wrap) ? wrap : RESET_TICKS;
    pwm_set_chan_level(clk_slice, clk_chan, pulse);

    // ---------- CV ----------
    float v_cv = (freq_hz * MV_PER_HZ) * CV_GAIN + CV_OFFSET;
    
    // Clamp voltage
    if (v_cv < 0.0f) v_cv = 0.0f;
    else if (v_cv > V_SUPPLY) v_cv = V_SUPPLY;

    // Convert to PWM level
    uint32_t level = (uint32_t)(v_cv * V_SUPPLY_INV * (cv_wrap + 1) + 0.5f);
    if (level > cv_wrap) level = cv_wrap;

    pwm_set_chan_level(cv_slice, cv_chan, level);
}

void synth_gate_on(void) {
    gpio_put(GATE_PIN, 1);
}

void synth_gate_off(void) {
    gpio_put(GATE_PIN, 0);
}