#include <stdio.h>
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

#include "synth.h"

int main() {
    stdio_init_all();
    board_init(); // TinyUSB board init
    tusb_init();  // TinyUSB stack init

    synth_init(); // Setup PWM

    // Play a startup tone so we know it's alive
    synth_set_frequency(440.0f);
    sleep_ms(200);
    synth_gate_off();

    while (true) {
        tud_task(); // TinyUSB device task
    }
}