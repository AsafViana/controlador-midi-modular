/**
 * Arduino mock for native testing environment.
 * Provides minimal stubs for Arduino functions used by BleServer.
 */
#pragma once

#include <cstdint>
#include <cstdio>

// ── Timing functions ─────────────────────────────────────────────────────────

static uint32_t _mock_millis_value = 0;

inline uint32_t millis() { return _mock_millis_value; }

inline void delay(uint32_t ms) { _mock_millis_value += ms; }

inline void mock_set_millis(uint32_t val) { _mock_millis_value = val; }
inline void mock_advance_millis(uint32_t delta) { _mock_millis_value += delta; }

// ── Logging macros (no-op in tests) ──────────────────────────────────────────

#define log_e(fmt, ...) ((void)0)
#define log_w(fmt, ...) ((void)0)
#define log_i(fmt, ...) ((void)0)
#define log_d(fmt, ...) ((void)0)
