/**
 * freertos/semphr.h – Simulation shim using pthreads mutexes.
 */

#ifndef SIM_FREERTOS_SEMPHR_H
#define SIM_FREERTOS_SEMPHR_H

#include "FreeRTOS.h"

/** Opaque handle – actually a pointer to pthread_mutex_t. */
typedef void *SemaphoreHandle_t;

SemaphoreHandle_t xSemaphoreCreateMutex();

BaseType_t xSemaphoreTake(SemaphoreHandle_t sem, TickType_t timeout);
BaseType_t xSemaphoreGive(SemaphoreHandle_t sem);

/**
 * ISR variants – in the simulation there are no real ISRs, so these
 * behave identically to the non-ISR versions.
 */
BaseType_t xSemaphoreTakeFromISR(SemaphoreHandle_t sem, BaseType_t *higherPrioWoken);
BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t sem, BaseType_t *higherPrioWoken);

#endif /* SIM_FREERTOS_SEMPHR_H */
