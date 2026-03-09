/**
 * Display – ILI9341 LCD + XPT2046 touch screen (LVGL-based).
 *
 * 320×240 ILI9341 SPI LCD via TFT_eSPI, LVGL 8 widget layer.
 * Two modes: scan (card-based vehicle selection) and control
 * (function bars, telemetry, trim).
 *
 * All LVGL calls must originate from a single display task thread.
 */

#pragma once

#include "../modules/ModuleManager.h"

#include <BatteryMonitor.h>
#include <Config.h>
#include <FunctionMap.h>
#include <Protocol.h>
#include <TelemetryData.h>
#include <TransmitterRadioLink.h>
#include <cstdint>
#include <lvgl.h>

namespace odh {

/** Touch event type communicated from LVGL callbacks to the main loop. */
enum class DisplayEvent : uint8_t {
    None       = 0,
    Vehicle    = 1, ///< Vehicle card tapped (index in eventData).
    Disconnect = 2, ///< Disconnect button tapped.
    TrimUp     = 3, ///< Trim increment (function idx in eventData).
    TrimDown   = 4, ///< Trim decrement (function idx in eventData).
};

class Display {
public:
    Display();

    /** Initialise LCD, touch, LVGL and build the widget tree. */
    bool begin();

    /** Update control screen (connected). */
    void refresh(const TelemetryData &telemetry, const BatteryMonitor &txBattery, const ModuleManager &moduleManager, const FunctionValue *functions, uint8_t functionCount);

    /** Update scan screen (scanning). */
    void refreshScan(const DiscoveredVehicle *vehicles, uint8_t count, uint8_t selectedIdx);

    bool isReady() const {
        return _ready;
    }

    /** Consume the last pending touch event. */
    DisplayEvent consumeEvent(uint8_t *outData = nullptr);

private:
    bool _ready    = false;
    bool _scanMode = true;

    // Pending touch event
    volatile DisplayEvent _pendingEvent = DisplayEvent::None;
    volatile uint8_t _pendingEventData  = 0;

    // LVGL draw buffers & drivers
    static lv_color_t _drawBuf[config::tx::kDisplayWidth * config::tx::kDisplayHeight / 10];
    static lv_disp_draw_buf_t _drawBufDesc;
    static lv_disp_drv_t _dispDrv;
    static lv_indev_drv_t _touchDrv;

    // Common header widgets
    lv_obj_t *_lblLinkBadge = nullptr;

    // Control mode widgets
    lv_obj_t *_controlPanel = nullptr;
    lv_obj_t *_lblTxBatt    = nullptr;
    lv_obj_t *_lblRxBatt    = nullptr;
    lv_obj_t *_rssiBar[4]   = {};
    lv_obj_t *_lblRssiVal   = nullptr;
    lv_obj_t *_lblPps       = nullptr;
    lv_obj_t *_lblUptime    = nullptr;
    bool _battShowPercent   = false;

    // Function rows (2-column, max 8)
    lv_obj_t *_chRowBg[8]     = {};
    lv_obj_t *_chBar[8]       = {};
    lv_obj_t *_chValLabel[8]  = {};
    lv_obj_t *_chNameLabel[8] = {};

    // Disconnect button
    lv_obj_t *_btnDisconnect = nullptr;

    // Trim overlay
    lv_obj_t *_trimPanel    = nullptr;
    lv_obj_t *_lblTrimFunc  = nullptr;
    lv_obj_t *_lblTrimVal   = nullptr;
    lv_obj_t *_btnTrimUp    = nullptr;
    lv_obj_t *_btnTrimDown  = nullptr;
    lv_obj_t *_btnTrimClose = nullptr;
    int8_t _selectedTrimIdx = -1;

    bool _battWarnBlink = false;

    // Scan mode widgets
    lv_obj_t *_scanPanel                       = nullptr;
    lv_obj_t *_scanCards[kMaxDiscovered]       = {};
    lv_obj_t *_scanModelIcon[kMaxDiscovered]   = {};
    lv_obj_t *_scanModelLabel[kMaxDiscovered]  = {};
    lv_obj_t *_scanNameLabel[kMaxDiscovered]   = {};
    lv_obj_t *_scanRssiBars[kMaxDiscovered][4] = {};
    lv_obj_t *_lblScanHint                     = nullptr;

    void buildUi();
    void showScanMode();
    void showControlMode();

    static void flushCb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p);
    static void touchReadCb(lv_indev_drv_t *drv, lv_indev_data_t *data);
    static void vehicleBtnCb(lv_event_t *e);
    static void disconnectBtnCb(lv_event_t *e);
    static void funcRowClickCb(lv_event_t *e);
    static void trimUpBtnCb(lv_event_t *e);
    static void trimDownBtnCb(lv_event_t *e);
    static void trimCloseBtnCb(lv_event_t *e);
    static void battTapCb(lv_event_t *e);

    static Display *_instance;
};

} // namespace odh
