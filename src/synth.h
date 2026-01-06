#ifndef SYNTH_H
#define SYNTH_H

#include <stdint.h>

// Initialize PWM hardware
void synth_init(void);

// Set the oscillator frequency
void synth_set_frequency(float freq_hz);

// Gate control for external VCA
void synth_gate_on(void);
void synth_gate_off(void);

#endif