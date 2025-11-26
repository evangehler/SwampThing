// usb_descriptors.c
#include "tusb.h"
#include <string.h>

// ----------------- Device Descriptor -----------------

tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,  // USB 2.0
    .bDeviceClass       = TUSB_CLASS_UNSPECIFIED,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCafe,  // Dummy VID
    .idProduct          = 0x4000,  // Dummy PID
    .bcdDevice          = 0x0100,  // v1.0

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

// TinyUSB asks for this
uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

// ----------------- Configuration Descriptor -----------------

enum {
    ITF_NUM_CONTROL = 0,
    ITF_NUM_MIDI,
    ITF_NUM_TOTAL
};

#define EPNUM_MIDI   0x01

// Total length = config descriptor + MIDI descriptor
#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)

uint8_t const desc_configuration[] = {
    // Config number, interfaces, string index, total length, attributes, power (mA)
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // MIDI interface: itf num, string index, EP OUT, EP IN, EP size
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EPNUM_MIDI, 0x80 | EPNUM_MIDI, 64)
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // only one configuration
    return desc_configuration;
}

// ----------------- String Descriptors -----------------

// 0: supported language is English (0x0409)
// 1: Manufacturer
// 2: Product
// 3: Serial
// 4: MIDI Interface
static char const *string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 }, // 0: LangID (English US)
    "AES",                          // 1: Manufacturer
    "SwampThing",                   // 2: Product
    "2026",                         // 3: Serial
    "SwampThing MIDI"              // 4: Interface
};

static uint16_t _desc_str[32];

uint16_t const * tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;

    uint8_t chr_count;

    if (index == 0) {
        // LangID = 0x0409
        _desc_str[1] = 0x0409;
        _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * 1 + 2);
        return _desc_str;
    }

    if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) {
        return NULL;
    }

    const char *str = string_desc_arr[index];

    // Cap to 31 UTF-16 code units
    chr_count = (uint8_t) strlen(str);
    if (chr_count > 31) chr_count = 31;

    for (uint8_t i = 0; i < chr_count; i++) {
        _desc_str[1 + i] = str[i];
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}
