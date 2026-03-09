/**
 * OpenDriveHub – Transmitter Firmware
 * Minimal entry point – all logic in TransmitterApp.
 */

#include "TransmitterApp.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static odh::TransmitterApp app;

void setup() {
    app.begin();
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
