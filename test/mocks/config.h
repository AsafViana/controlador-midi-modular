#pragma once

// Hardware
#define USB_MIDI_DEVICE_NAME  "Controlador MIDI"

// MIDI
#define MIDI_DEFAULT_VELOCITY 100
#define MIDI_DEFAULT_CHANNEL  1
#define MIDI_DEFAULT_CC_VALUE  0

// MIDI DIN (serial 5-pin) — não usado no build native, mas definido para compilar
#define MIDI_DIN_BAUD         31250
#define MIDI_DIN_TX_PIN       17

// Display OLED SSD1306
#define DISPLAY_I2C_ADDRESS 0x3C
#define DISPLAY_WIDTH       128
#define DISPLAY_HEIGHT      64
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define HEADER_HEIGHT       16
#define CONTENT_Y           HEADER_HEIGHT
#define CONTENT_HEIGHT      (OLED_HEIGHT - HEADER_HEIGHT)
