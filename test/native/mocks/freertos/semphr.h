/**
 * FreeRTOS semaphore/mutex mock for native testing environment.
 * Provides pass-through stubs (no actual synchronization needed in
 * single-threaded native tests).
 */
#pragma once

#include "FreeRTOS.h"

// Semaphore handle is just a pointer in the mock
typedef void *SemaphoreHandle_t;

// Mock implementations — always succeed immediately
inline SemaphoreHandle_t xSemaphoreCreateMutex() {
  static int fakeMutex = 1;
  return (SemaphoreHandle_t)&fakeMutex;
}

inline BaseType_t xSemaphoreTake(SemaphoreHandle_t, TickType_t) {
  return pdTRUE;
}

inline BaseType_t xSemaphoreGive(SemaphoreHandle_t) { return pdTRUE; }
