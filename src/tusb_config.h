#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------
// COMMON CONFIGURATION
// --------------------------------------------------------------------

#ifndef CFG_TUSB_MCU
  #define CFG_TUSB_MCU    OPT_MCU_RP2040
#endif

#define CFG_TUSB_RHPORT0_MODE   OPT_MODE_DEVICE

// --------------------------------------------------------------------
// DEVICE CONFIGURATION
// --------------------------------------------------------------------

#define CFG_TUD_ENABLED       1

// Enable MIDI Class
#define CFG_TUD_MIDI          1

// Disable other
#define CFG_TUD_CDC           0
#define CFG_TUD_MSC           0
#define CFG_TUD_HID           0
#define CFG_TUD_VENDOR        0

// MIDI Buffer sizes (64 bytes)
#define CFG_TUD_MIDI_RX_BUFSIZE 64
#define CFG_TUD_MIDI_TX_BUFSIZE 64

// Endpoint 0 Buffer Size (Control endpoint)
#define CFG_TUD_ENDPOINT0_SIZE  64

#ifdef __cplusplus
}
#endif

#endif