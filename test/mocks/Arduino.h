#pragma once
#include <cstdint>
#include <cstddef>

// --- Pin modes ---
#define INPUT         0x0
#define OUTPUT        0x1
#define INPUT_PULLUP  0x2
#define INPUT_PULLDOWN 0x3

#define HIGH 1
#define LOW  0

// --- Mock state ---
namespace mock {
    extern uint32_t currentMillis;
    extern int      pinModes[256];
    extern int      pinValues[256];
    extern int      delayCalledMs;

    void reset();
    void setMillis(uint32_t ms);
    void advanceMillis(uint32_t ms);
    void setDigitalRead(uint8_t pin, int value);
}

// --- Arduino API stubs ---
inline uint32_t millis() { return mock::currentMillis; }

inline void pinMode(uint8_t pin, uint8_t mode) {
    mock::pinModes[pin] = mode;
}

inline int digitalRead(uint8_t pin) {
    return mock::pinValues[pin];
}

inline void digitalWrite(uint8_t pin, uint8_t val) {
    mock::pinValues[pin] = val;
}

inline void delay(unsigned long ms) {
    mock::delayCalledMs = static_cast<int>(ms);
    mock::currentMillis += ms;
}
