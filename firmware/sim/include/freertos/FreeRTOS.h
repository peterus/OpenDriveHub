/**
 * freertos/FreeRTOS.h – Simulation shim.
 *
 * Provides FreeRTOS base types and macros used by the firmware.
 * Actual task/semaphore implementations are in task.h and semphr.h.
 */

#ifndef SIM_FREERTOS_H
#define SIM_FREERTOS_H

#include <cstdint>

typedef int32_t BaseType_t;
typedef uint32_t UBaseType_t;
typedef uint32_t TickType_t;

#define pdTRUE  1
#define pdFALSE 0
#define pdPASS  pdTRUE
#define pdFAIL  pdFALSE

/** Assume 1 ms per tick for the simulation. */
#define configTICK_RATE_HZ 1000

#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
#define pdTICKS_TO_MS(t)  ((uint32_t)(t))

#define portMAX_DELAY 0xFFFFFFFF

#endif /* SIM_FREERTOS_H */
