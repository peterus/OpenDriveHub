/**
 * ReceiverApp – top-level application class for the receiver firmware.
 *
 * Owns all subsystems and FreeRTOS tasks:
 *   - OutputManager (PCA9685 or logging drivers)
 *   - ReceiverRadioLink (ESP-NOW)
 *   - BatteryMonitor (local ADC)
 *   - OdhWebServer + ReceiverApi (REST config)
 *
 * FreeRTOS tasks:
 *   taskOutput    – applies channel values at 50 Hz, checks failsafe/link timeout
 *   taskTelemetry – sends telemetry to transmitter at 10 Hz
 *   taskWeb       – runs web server for configuration (if config button held)
 */

#pragma once

#include "BatteryMonitor.h"
#include "Config.h"
#include "OdhWebServer.h"
#include "OutputManager.h"
#include "ReceiverRadioLink.h"
#include "web/ReceiverApi.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

namespace odh {

class ReceiverApp {
public:
    ReceiverApp();
    ~ReceiverApp() = default;

    /// Initialise all subsystems and start FreeRTOS tasks.
    void begin();

private:
    OutputManager _output;
    ReceiverRadioLink _radio;
    BatteryMonitor _battery;
    OdhWebServer _webServer;
    ReceiverApi _api;

    SemaphoreHandle_t _channelsMux = nullptr;
    bool _failsafeActive           = false;
    bool _webConfigMode            = false;

    // Vehicle config (loaded from NVS).
    char _vehicleName[kVehicleNameMax] = {};

    // Control packet callback.
    void onControlReceived(const ControlPacket &pkt);

    // FreeRTOS task functions.
    static void taskOutputFn(void *param);
    static void taskTelemetryFn(void *param);
    static void taskWebFn(void *param);

    void runOutputLoop();
    void runTelemetryLoop();
    void runWebLoop();

    /// Load vehicle name from NVS.
    void loadVehicleName();
    void saveVehicleName();

    /// Check if config button is held at boot.
    bool isConfigButtonPressed() const;
};

} // namespace odh
