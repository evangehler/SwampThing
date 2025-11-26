#include "synth.h"
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define CLOCK_PIN 13
#define CV_PIN    14
#define SYS_CLK   125000000.0f
#define V_SUPPLY  3.3f
#define MV_PER_HZ 0.00100f 

// --- STATIC VARs ---
static uint  clk_slice, clk_chan;
static uint  cv_slice,  cv_chan;
static const uint16_t CLOCK_WRAP = 65535;
static uint32_t cv_wrap_config = 0; 

void synth_init(void) {
    // CLOCK PWM
    gpio_set_function(CLOCK_PIN, GPIO_FUNC_PWM);
    clk_slice = pwm_gpio_to_slice_num(CLOCK_PIN);
    clk_chan  = pwm_gpio_to_channel(CLOCK_PIN);


    pwm_config clk_cfg = pwm_get_default_config();
    pwm_config_set_wrap(&clk_cfg, CLOCK_WRAP);
    pwm_init(clk_slice, &clk_cfg, true);
    pwm_set_chan_level(clk_slice, clk_chan, CLOCK_WRAP / 2);

    // CV PWM
    gpio_set_function(CV_PIN, GPIO_FUNC_PWM);
    cv_slice = pwm_gpio_to_slice_num(CV_PIN);
    cv_chan  = pwm_gpio_to_channel(CV_PIN);

    pwm_config cv_cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cv_cfg, 4.0f);
    pwm_config_set_wrap(&cv_cfg, 4095);      // ~12-bit CV
    pwm_init(cv_slice, &cv_cfg, true);
    cv_wrap_config = 4095;
}

void synth_set_frequency(float freq_hz) {
    // f = SYS_CLK / (clkdiv * (CLOCK_WRAP+1))
    float div = SYS_CLK / (freq_hz * (CLOCK_WRAP + 1));

    if (div < 1.0f)   div = 1.0f;    // max PWM rate
    if (div > 256.0f) div = 256.0f;  // RP2040 limit

    pwm_set_clkdiv(clk_slice, div);
    pwm_set_chan_level(clk_slice, clk_chan, (CLOCK_WRAP + 1) / 2);

   // ----- CV ------
    float v_cv = freq_hz * MV_PER_HZ;
    if (v_cv > V_SUPPLY) v_cv = V_SUPPLY;
    if (v_cv < 0.0f)     v_cv = 0.0f;

    float duty = v_cv / V_SUPPLY;
    pwm_set_chan_level(cv_slice, cv_chan,
                       (uint32_t)(cv_wrap_config * duty));
}


void synth_gate_off(void) {
    // later
}