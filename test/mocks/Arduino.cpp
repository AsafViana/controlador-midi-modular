#include "Arduino.h"
#include <cstring>

namespace mock {
    uint32_t currentMillis = 0;
    int      pinModes[256];
    int      pinValues[256];
    int      delayCalledMs = -1;

    void reset() {
        currentMillis = 0;
        delayCalledMs = -1;
        memset(pinModes, 0, sizeof(pinModes));
        memset(pinValues, 0, sizeof(pinValues));
    }

    void setMillis(uint32_t ms) {
        currentMillis = ms;
    }

    void advanceMillis(uint32_t ms) {
        currentMillis += ms;
    }

    void setDigitalRead(uint8_t pin, int value) {
        pinValues[pin] = value;
    }
}
