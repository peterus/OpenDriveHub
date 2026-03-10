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
