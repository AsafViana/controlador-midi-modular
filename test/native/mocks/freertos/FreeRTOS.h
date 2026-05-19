/**
 * FreeRTOS mock for native testing environment.
 * Provides minimal stubs for FreeRTOS types and macros used by CCStateStore.
 */
#pragma once

#include <cstdint>

// FreeRTOS base types
typedef int BaseType_t;
typedef uint32_t TickType_t;

#define pdTRUE 1
#define pdFALSE 0
#define pdPASS pdTRUE
#define pdFAIL pdFALSE

#define pdMS_TO_TICKS(xTimeInMs) ((TickType_t)(xTimeInMs))

#define portMAX_DELAY ((TickType_t)0xFFFFFFFF)
