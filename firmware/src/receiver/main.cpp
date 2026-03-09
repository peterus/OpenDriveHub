/**
 * OpenDriveHub – Receiver Firmware Entry Point
 *
 * Minimal main.cpp – all logic lives in ReceiverApp.
 */

#include "ReceiverApp.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static odh::ReceiverApp app;

void setup() {
    app.begin();
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
