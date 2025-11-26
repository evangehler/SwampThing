#include "usb_midi.h"
#include "tusb.h"
#include "synth.h"
#include <math.h>

#define A4_FREQ 440.0f
#define A4_MIDI 69

int8_t active_note = -1;

float midi_to_freq(uint8_t note) {
    return A4_FREQ * powf(2.0f, (float)(note - A4_MIDI) / 12.0f);
}

// TinyUSB when MIDI data is received
void tud_midi_rx_cb(uint8_t itf) {
    uint8_t packet[4];
    while (tud_midi_packet_read(packet)) {
        uint8_t status = packet[1] & 0xF0;
        uint8_t note   = packet[2];
        uint8_t vel    = packet[3];

        if (status == 0x90 && vel > 0) {
            float freq = midi_to_freq(note);
            synth_set_frequency(freq);
            active_note = note;
        }
        else if (status == 0x80 || (status == 0x90 && vel == 0)) {
            if (note == active_note) {
                synth_gate_off();
                active_note = -1;
            }
        }
    }
}