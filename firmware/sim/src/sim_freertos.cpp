/*
 * Copyright (C) 2026 Peter Buchegger
 *
 * This file is part of OpenDriveHub.
 *
 * OpenDriveHub is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenDriveHub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenDriveHub. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * sim_freertos.cpp – FreeRTOS API implementation using pthreads.
 */

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include <chrono>
#include <pthread.h>
#include <thread>

/* ── Task API ───────────────────────────────────────────────────────────── */

TickType_t xTaskGetTickCount() {
    return static_cast<TickType_t>(millis());
}

void vTaskDelay(TickType_t ticks) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ticks));
}

void vTaskDelayUntil(TickType_t *previousWakeTime, TickType_t increment) {
    TickType_t now  = xTaskGetTickCount();
    TickType_t next = *previousWakeTime + increment;

    if (next > now) {
        std::this_thread::sleep_for(std::chrono::milliseconds(next - now));
    }
    *previousWakeTime = xTaskGetTickCount();
}

struct TaskParams {
    TaskFunction_t fn;
    void *param;
};

static void *taskWrapper(void *arg) {
    auto *p = static_cast<TaskParams *>(arg);
    p->fn(p->param);
    delete p;
    return nullptr;
}

BaseType_t xTaskCreatePinnedToCore(TaskFunction_t fn, const char *, uint32_t, void *param, UBaseType_t, TaskHandle_t *handle, BaseType_t) {
    auto *p  = new TaskParams{fn, param};
    auto *th = new pthread_t;
    int ret  = pthread_create(th, nullptr, taskWrapper, p);
    if (handle) {
        *handle = th;
    }
    return ret == 0 ? pdPASS : pdFAIL;
}

/* ── Semaphore API ──────────────────────────────────────────────────────── */

SemaphoreHandle_t xSemaphoreCreateMutex() {
    auto *mtx = new pthread_mutex_t;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return static_cast<SemaphoreHandle_t>(mtx);
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t sem, TickType_t timeout) {
    if (!sem)
        return pdFALSE;
    auto *mtx = static_cast<pthread_mutex_t *>(sem);

    if (timeout == portMAX_DELAY) {
        return pthread_mutex_lock(mtx) == 0 ? pdTRUE : pdFALSE;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t ns = static_cast<uint64_t>(timeout) * 1000000ULL;
    ts.tv_nsec += ns % 1000000000ULL;
    ts.tv_sec += ns / 1000000000ULL;
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }
    return pthread_mutex_timedlock(mtx, &ts) == 0 ? pdTRUE : pdFALSE;
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t sem) {
    if (!sem)
        return pdFALSE;
    return pthread_mutex_unlock(static_cast<pthread_mutex_t *>(sem)) == 0 ? pdTRUE : pdFALSE;
}

BaseType_t xSemaphoreTakeFromISR(SemaphoreHandle_t sem, BaseType_t *higherPrioWoken) {
    if (higherPrioWoken)
        *higherPrioWoken = pdFALSE;
    return xSemaphoreTake(sem, 0);
}

BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t sem, BaseType_t *higherPrioWoken) {
    if (higherPrioWoken)
        *higherPrioWoken = pdFALSE;
    return xSemaphoreGive(sem);
}
