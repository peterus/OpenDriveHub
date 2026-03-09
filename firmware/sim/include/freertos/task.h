/**
 * freertos/task.h – Simulation shim using pthreads.
 */

#ifndef SIM_FREERTOS_TASK_H
#define SIM_FREERTOS_TASK_H

#include "FreeRTOS.h"

typedef void *TaskHandle_t;
typedef void (*TaskFunction_t)(void *);

TickType_t xTaskGetTickCount();

void vTaskDelay(TickType_t ticks);
void vTaskDelayUntil(TickType_t *previousWakeTime, TickType_t increment);

BaseType_t xTaskCreatePinnedToCore(TaskFunction_t fn, const char *name,
                                   uint32_t stackDepth, void *param,
                                   UBaseType_t priority, TaskHandle_t *handle,
                                   BaseType_t core);

#endif /* SIM_FREERTOS_TASK_H */
