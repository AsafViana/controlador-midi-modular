#pragma once

// Hardware
#define USB_MIDI_DEVICE_NAME  "Controlador MIDI"

// MIDI
#define MIDI_DEFAULT_VELOCITY 100
#define MIDI_DEFAULT_CHANNEL  1
#define MIDI_DEFAULT_CC_VALUE  0

// Display OLED SSD1306
#define DISPLAY_I2C_ADDRESS   0x3C
#define OLED_WIDTH            128
#define OLED_HEIGHT           64
#define HEADER_HEIGHT         16   // Faixa amarela: Y 0-15
#define CONTENT_Y             HEADER_HEIGHT  // Início da área azul
#define CONTENT_HEIGHT        (OLED_HEIGHT - HEADER_HEIGHT)  // 48px
