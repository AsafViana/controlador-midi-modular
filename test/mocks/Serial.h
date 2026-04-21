#pragma once
#include <cstdint>
#include <cstring>

/**
 * Minimal mock of Arduino Serial for native testing.
 * Captures printed messages for test assertions.
 */
class MockSerial {
public:
    static constexpr size_t BUFFER_SIZE = 256;
    char lastMessage[BUFFER_SIZE] = {};
    int printlnCallCount = 0;

    void begin(unsigned long) {}

    void println(const char* msg) {
        printlnCallCount++;
        if (msg) {
            strncpy(lastMessage, msg, BUFFER_SIZE - 1);
            lastMessage[BUFFER_SIZE - 1] = '\0';
        }
    }

    void print(const char*) {}
    void print(int) {}
    void println(int) {}
    void println() { printlnCallCount++; }

    void reset() {
        printlnCallCount = 0;
        lastMessage[0] = '\0';
    }
};

extern MockSerial Serial;
