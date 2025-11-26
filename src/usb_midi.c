#include "usb_midi.h"
#include "tusb.h"
#include "synth.h"
#include <math.h>

#include "pico/stdlib.h"
#include "bsp/board.h"

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

        // Handle Note On/Off
        if (status == 0x90 && vel > 0) {
            // Note On: set frequency and turn LED on
            float freq = midi_to_freq(note);
            synth_set_frequency(freq);
            synth_gate_on();
            board_led_write(true);
            active_note = note;

        }
        else if (status == 0x80 || (status == 0x90 && vel == 0)) {
            // Note Off
            if (note == active_note) {
                synth_gate_off();
                board_led_write(false);
                active_note = -1;
               
            }
        }
    }
}