/**
 * Display – implementation.
 *
 * Modernised: uses odh:: namespace, enum class, Config.h constants,
 * FunctionMap.h helpers.  Logic and layout are unchanged from the
 * original 1012-line file.
 */

#include "Display.h"

#include <Arduino.h>
#include <Preferences.h>

#include "display_utils.h"

#include <FunctionMap.h>

#ifdef NATIVE_SIM
#include <SDL2/SDL.h>
#include <sim_keyboard.h>
#else
#include <TFT_eSPI.h>
#endif

namespace odh {

/* ── Colour palette ──────────────────────────────────────────────────────── */

static constexpr uint32_t COL_BG        = 0x0D1117;
static constexpr uint32_t COL_HEADER_BG = 0x1565C0;
static constexpr uint32_t COL_PANEL_BG  = 0x161B22;
static constexpr uint32_t COL_TEXT      = 0xE6EDF3;
static constexpr uint32_t COL_TEXT_DIM  = 0x8B949E;
static constexpr uint32_t COL_CONNECTED = 0x3FB950;
static constexpr uint32_t COL_BINDING   = 0xD29922;
static constexpr uint32_t COL_FAILSAFE  = 0xF85149;
static constexpr uint32_t COL_DISC      = 0x6E7681;
static constexpr uint32_t COL_BAR_BG    = 0x21262D;
static constexpr uint32_t COL_BAR_FG    = 0x1F6FEB;
static constexpr uint32_t COL_BAR_MID   = 0x3FB950;
static constexpr uint32_t COL_BTN_BG    = 0x1F6FEB;
static constexpr uint32_t COL_BTN_RED   = 0xF85149;
static constexpr uint32_t COL_BATT_OK   = 0x3FB950;
static constexpr uint32_t COL_BATT_WARN = 0xD29922;
static constexpr uint32_t COL_BATT_CRIT = 0xF85149;
static constexpr uint32_t COL_RSSI_ON   = 0x3FB950;
static constexpr uint32_t COL_RSSI_OFF  = 0x21262D;
static constexpr uint32_t COL_CARD_BG   = 0x1C2128;
static constexpr uint32_t COL_TRIM_BG   = 0x1C2128;

static constexpr uint32_t COL_MODEL_GENERIC = 0x6E7681;
static constexpr uint32_t COL_MODEL_DUMP    = 0x1F6FEB;
static constexpr uint32_t COL_MODEL_EXCAV   = 0xD29922;
static constexpr uint32_t COL_MODEL_TRACT   = 0x3FB950;
static constexpr uint32_t COL_MODEL_CRANE   = 0xA371F7;

/* ── Layout constants (landscape 320×240) ────────────────────────────────── */

static constexpr int16_t SCR_W = config::tx::kDisplayWidth;
static constexpr int16_t SCR_H = config::tx::kDisplayHeight;

static constexpr int16_t HDR_H  = 36;
static constexpr int16_t INFO_H = 28;
static constexpr int16_t CH_H   = 28;

static constexpr int16_t COL_W      = SCR_W / 2;
static constexpr int16_t COL_NAME_W = 80;
static constexpr int16_t COL_BAR_X  = 84;
static constexpr int16_t COL_BAR_W  = COL_W - COL_BAR_X - 4;
static constexpr int16_t CH_BAR_H   = 14;

static constexpr int16_t FUNC_ROWS  = 4;
static constexpr int16_t DISC_BAR_H = 28;
static constexpr int16_t TRIM_H     = 36;

static constexpr int16_t SCAN_CARD_W   = SCR_W - 24;
static constexpr int16_t SCAN_CARD_H   = 42;
static constexpr int16_t SCAN_CARD_GAP = 4;
static constexpr int16_t SCAN_ICON_SZ  = 28;

/* ── File-scope TFT / SDL state ──────────────────────────────────────────── */
#ifdef NATIVE_SIM
static SDL_Window *s_sdlWindow     = nullptr;
static SDL_Renderer *s_sdlRenderer = nullptr;
static SDL_Texture *s_sdlTexture   = nullptr;
static bool s_mousePressed         = false;
static int s_mouseX                = 0;
static int s_mouseY                = 0;
#else
static TFT_eSPI s_tft;
#endif

/* ── Static LVGL resources ───────────────────────────────────────────────── */
lv_color_t Display::_drawBuf[config::tx::kDisplayWidth * config::tx::kDisplayHeight / 10];
lv_disp_draw_buf_t Display::_drawBufDesc;
lv_disp_drv_t Display::_dispDrv;
lv_indev_drv_t Display::_touchDrv;
Display *Display::_instance = nullptr;

/* ── Helpers ─────────────────────────────────────────────────────────────── */

static uint8_t rssiToBars(int8_t rssi) {
    if (rssi > -50)
        return 4;
    if (rssi > -65)
        return 3;
    if (rssi > -75)
        return 2;
    if (rssi > -85)
        return 1;
    return 0;
}

static uint32_t modelIconColor(uint8_t modelType) {
    switch (static_cast<ModelType>(modelType)) {
    case ModelType::DumpTruck:
        return COL_MODEL_DUMP;
    case ModelType::Excavator:
        return COL_MODEL_EXCAV;
    case ModelType::Tractor:
        return COL_MODEL_TRACT;
    case ModelType::Crane:
        return COL_MODEL_CRANE;
    default:
        return COL_MODEL_GENERIC;
    }
}

static const char *modelIconLetter(uint8_t modelType) {
    switch (static_cast<ModelType>(modelType)) {
    case ModelType::DumpTruck:
        return "D";
    case ModelType::Excavator:
        return "E";
    case ModelType::Tractor:
        return "T";
    case ModelType::Crane:
        return "C";
    default:
        return "G";
    }
}

static uint32_t battCellColor(uint16_t cellMv) {
    if (cellMv == 0)
        return COL_TEXT_DIM;
    if (cellMv <= 3300)
        return COL_BATT_CRIT;
    if (cellMv <= 3500)
        return COL_BATT_WARN;
    return COL_BATT_OK;
}

static lv_obj_t *makePanel(lv_obj_t *parent, int16_t w, int16_t h, int16_t x, int16_t y, uint32_t bgCol) {
    lv_obj_t *p = lv_obj_create(parent);
    lv_obj_set_size(p, w, h);
    lv_obj_set_pos(p, x, y);
    lv_obj_set_style_bg_color(p, lv_color_hex(bgCol), 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(p, 0, 0);
    lv_obj_set_style_radius(p, 0, 0);
    lv_obj_set_style_pad_all(p, 0, 0);
    lv_obj_clear_flag(p, LV_OBJ_FLAG_SCROLLABLE);
    return p;
}

/* ── Constructor ─────────────────────────────────────────────────────────── */
Display::Display() = default;

/* ── begin() ─────────────────────────────────────────────────────────────── */
bool Display::begin() {
    _instance = this;

#ifdef NATIVE_SIM
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[SIM] SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    s_sdlWindow = SDL_CreateWindow("OpenDriveHub TX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, config::tx::kDisplayWidth * 2, config::tx::kDisplayHeight * 2, SDL_WINDOW_SHOWN);
    if (!s_sdlWindow) {
        printf("[SIM] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }
#else
    s_tft.begin();
    s_tft.setRotation(1);
    s_tft.fillScreen(TFT_BLACK);

    pinMode(config::tx::kLcdBlPin, OUTPUT);
    digitalWrite(config::tx::kLcdBlPin, config::tx::kLcdBlActiveHigh ? HIGH : LOW);

    // Touch calibration
    {
        Preferences tp;
        tp.begin("odh", false);
        uint16_t calData[5] = {};
        size_t stored       = tp.getBytes("touch_cal", calData, sizeof(calData));
        if (stored != sizeof(calData)) {
            s_tft.fillScreen(TFT_BLACK);
            s_tft.setTextColor(TFT_WHITE, TFT_BLACK);
            s_tft.setTextDatum(MC_DATUM);
            s_tft.drawString("Touch calibration", config::tx::kDisplayWidth / 2, config::tx::kDisplayHeight / 2 - 16, 2);
            s_tft.drawString("Tap the corners", config::tx::kDisplayWidth / 2, config::tx::kDisplayHeight / 2 + 8, 2);
            delay(1500);
            s_tft.calibrateTouch(calData, TFT_WHITE, TFT_BLACK, 20);
            tp.putBytes("touch_cal", calData, sizeof(calData));
            s_tft.fillScreen(TFT_BLACK);
        }
        s_tft.setTouch(calData);
        tp.end();
    }
#endif

    lv_init();

    lv_disp_draw_buf_init(&_drawBufDesc, _drawBuf, nullptr, config::tx::kDisplayWidth * config::tx::kDisplayHeight / 10);

    lv_disp_drv_init(&_dispDrv);
    _dispDrv.hor_res  = config::tx::kDisplayWidth;
    _dispDrv.ver_res  = config::tx::kDisplayHeight;
    _dispDrv.flush_cb = flushCb;
    _dispDrv.draw_buf = &_drawBufDesc;
    lv_disp_drv_register(&_dispDrv);

    lv_indev_drv_init(&_touchDrv);
    _touchDrv.type    = LV_INDEV_TYPE_POINTER;
    _touchDrv.read_cb = touchReadCb;
    lv_indev_drv_register(&_touchDrv);

    // Load battery display preference
    {
        Preferences bp;
        bp.begin("odh", true);
        _battShowPercent = bp.getUChar("batt_pct", 0) != 0;
        bp.end();
    }

    buildUi();
    _ready = true;
    return true;
}

/* ── consumeEvent() ──────────────────────────────────────────────────────── */
DisplayEvent Display::consumeEvent(uint8_t *outData) {
    auto ev = _pendingEvent;
    if (ev != DisplayEvent::None) {
        if (outData)
            *outData = _pendingEventData;
        _pendingEvent     = DisplayEvent::None;
        _pendingEventData = 0;
    }
    return ev;
}

/* ── flushCb() ───────────────────────────────────────────────────────────── */
void Display::flushCb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
#ifdef NATIVE_SIM
    if (!s_sdlRenderer) {
        s_sdlRenderer = SDL_CreateRenderer(s_sdlWindow, -1, SDL_RENDERER_ACCELERATED);
        if (!s_sdlRenderer)
            s_sdlRenderer = SDL_CreateRenderer(s_sdlWindow, -1, SDL_RENDERER_SOFTWARE);
        s_sdlTexture = SDL_CreateTexture(s_sdlRenderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, config::tx::kDisplayWidth, config::tx::kDisplayHeight);
    }
    if (s_sdlTexture) {
        SDL_Rect rect{area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1};
        SDL_UpdateTexture(s_sdlTexture, &rect, color_p, rect.w * sizeof(lv_color_t));
        SDL_RenderCopy(s_sdlRenderer, s_sdlTexture, nullptr, nullptr);
        SDL_RenderPresent(s_sdlRenderer);
    }
#else
    uint32_t w = static_cast<uint32_t>(area->x2 - area->x1 + 1);
    uint32_t h = static_cast<uint32_t>(area->y2 - area->y1 + 1);
    s_tft.startWrite();
    s_tft.setAddrWindow(area->x1, area->y1, w, h);
    s_tft.pushColors(reinterpret_cast<uint16_t *>(color_p), w * h, true);
    s_tft.endWrite();
#endif
    lv_disp_flush_ready(drv);
}

/* ── touchReadCb() ───────────────────────────────────────────────────────── */
void Display::touchReadCb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    (void)drv;
#ifdef NATIVE_SIM
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            exit(0);
            break;
        case SDL_MOUSEBUTTONDOWN:
            s_mousePressed = true;
            s_mouseX       = event.button.x / 2;
            s_mouseY       = event.button.y / 2;
            break;
        case SDL_MOUSEBUTTONUP:
            s_mousePressed = false;
            break;
        case SDL_MOUSEMOTION:
            if (s_mousePressed) {
                s_mouseX = event.motion.x / 2;
                s_mouseY = event.motion.y / 2;
            }
            break;
        case SDL_KEYDOWN: {
            auto &kb = g_simKeyboard;
            std::lock_guard<std::mutex> lock(kb.mtx);
            auto sym = event.key.keysym.sym;
            if (sym >= SDLK_1 && sym <= SDLK_8) {
                kb.activeChannel = static_cast<uint8_t>(sym - SDLK_1);
            } else if (sym == SDLK_UP) {
                auto &ch = kb.channels[kb.activeChannel];
                ch       = (ch + 10 > 2000) ? 2000 : ch + 10;
            } else if (sym == SDLK_DOWN) {
                auto &ch = kb.channels[kb.activeChannel];
                ch       = (ch < 1010) ? 1000 : ch - 10;
            } else if (sym == SDLK_PAGEUP) {
                auto &ch = kb.channels[kb.activeChannel];
                ch       = (ch + 100 > 2000) ? 2000 : ch + 100;
            } else if (sym == SDLK_PAGEDOWN) {
                auto &ch = kb.channels[kb.activeChannel];
                ch       = (ch < 1100) ? 1000 : ch - 100;
            } else if (sym == SDLK_HOME) {
                kb.channels[kb.activeChannel] = 1500;
            } else if (sym == SDLK_SPACE) {
                std::fill(std::begin(kb.channels), std::end(kb.channels), static_cast<uint16_t>(1500));
            }
            break;
        }
        default:
            break;
        }
    }
    data->state   = s_mousePressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point.x = static_cast<lv_coord_t>(s_mouseX);
    data->point.y = static_cast<lv_coord_t>(s_mouseY);
#else
    uint16_t tx = 0, ty = 0;
    bool pressed  = s_tft.getTouch(&tx, &ty);
    data->state   = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point.x = static_cast<lv_coord_t>(tx);
    data->point.y = static_cast<lv_coord_t>(ty);
#endif
}

/* ── Button event callbacks ──────────────────────────────────────────────── */

void Display::vehicleBtnCb(lv_event_t *e) {
    if (!_instance)
        return;
    auto idx                     = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
    _instance->_pendingEvent     = DisplayEvent::Vehicle;
    _instance->_pendingEventData = idx;
}

void Display::disconnectBtnCb(lv_event_t *e) {
    (void)e;
    if (!_instance)
        return;
    _instance->_pendingEvent     = DisplayEvent::Disconnect;
    _instance->_pendingEventData = 0;
}

void Display::funcRowClickCb(lv_event_t *e) {
    if (!_instance)
        return;
    auto idx                    = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
    _instance->_selectedTrimIdx = static_cast<int8_t>(idx);
    lv_obj_clear_flag(_instance->_trimPanel, LV_OBJ_FLAG_HIDDEN);
}

void Display::trimUpBtnCb(lv_event_t *e) {
    (void)e;
    if (!_instance || _instance->_selectedTrimIdx < 0)
        return;
    _instance->_pendingEvent     = DisplayEvent::TrimUp;
    _instance->_pendingEventData = static_cast<uint8_t>(_instance->_selectedTrimIdx);
}

void Display::trimDownBtnCb(lv_event_t *e) {
    (void)e;
    if (!_instance || _instance->_selectedTrimIdx < 0)
        return;
    _instance->_pendingEvent     = DisplayEvent::TrimDown;
    _instance->_pendingEventData = static_cast<uint8_t>(_instance->_selectedTrimIdx);
}

void Display::trimCloseBtnCb(lv_event_t *e) {
    (void)e;
    if (!_instance)
        return;
    _instance->_selectedTrimIdx = -1;
    lv_obj_add_flag(_instance->_trimPanel, LV_OBJ_FLAG_HIDDEN);
}

void Display::battTapCb(lv_event_t *e) {
    (void)e;
    if (!_instance)
        return;
    _instance->_battShowPercent = !_instance->_battShowPercent;
    Preferences p;
    p.begin("odh", false);
    p.putUChar("batt_pct", _instance->_battShowPercent ? 1 : 0);
    p.end();
}

/* ── buildUi() ───────────────────────────────────────────────────────────── */

void Display::buildUi() {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(COL_BG), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Header bar */
    lv_obj_t *hdr = makePanel(scr, SCR_W, HDR_H, 0, 0, COL_HEADER_BG);

    lv_obj_t *lblTitle = lv_label_create(hdr);
    lv_label_set_text(lblTitle, "OpenDriveHub");
    lv_obj_set_style_text_color(lblTitle, lv_color_hex(COL_TEXT), 0);
    lv_obj_set_style_text_font(lblTitle, &lv_font_montserrat_16, 0);
    lv_obj_align(lblTitle, LV_ALIGN_LEFT_MID, 10, 0);

    _lblLinkBadge = lv_label_create(hdr);
    lv_label_set_text(_lblLinkBadge, "---");
    lv_obj_set_style_text_color(_lblLinkBadge, lv_color_hex(COL_DISC), 0);
    lv_obj_set_style_text_font(_lblLinkBadge, &lv_font_montserrat_14, 0);
    lv_obj_align(_lblLinkBadge, LV_ALIGN_RIGHT_MID, -10, 0);

    /* Control panel */
    _controlPanel = makePanel(scr, SCR_W, SCR_H - HDR_H, 0, HDR_H, COL_BG);

    /* Info bar */
    lv_obj_t *info = makePanel(_controlPanel, SCR_W, INFO_H, 0, 0, COL_PANEL_BG);

    _lblTxBatt = lv_label_create(info);
    lv_label_set_text(_lblTxBatt, "TX: -.-V");
    lv_obj_set_style_text_color(_lblTxBatt, lv_color_hex(COL_TEXT), 0);
    lv_obj_set_style_text_font(_lblTxBatt, &lv_font_montserrat_12, 0);
    lv_obj_align(_lblTxBatt, LV_ALIGN_LEFT_MID, 6, 0);
    lv_obj_add_flag(_lblTxBatt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_lblTxBatt, battTapCb, LV_EVENT_CLICKED, nullptr);

    _lblRxBatt = lv_label_create(info);
    lv_label_set_text(_lblRxBatt, "RX: ---");
    lv_obj_set_style_text_color(_lblRxBatt, lv_color_hex(COL_TEXT_DIM), 0);
    lv_obj_set_style_text_font(_lblRxBatt, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblRxBatt, 72, (INFO_H - 12) / 2);
    lv_obj_add_flag(_lblRxBatt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_lblRxBatt, battTapCb, LV_EVENT_CLICKED, nullptr);

    /* RSSI bars */
    static const int16_t rssiBarH[4] = {5, 9, 13, 17};
    for (uint8_t b = 0; b < 4; ++b) {
        _rssiBar[b] = lv_obj_create(info);
        int16_t bx  = 140 + b * 7;
        int16_t bh  = rssiBarH[b];
        lv_obj_set_size(_rssiBar[b], 5, bh);
        lv_obj_set_pos(_rssiBar[b], bx, INFO_H - bh - 4);
        lv_obj_set_style_bg_color(_rssiBar[b], lv_color_hex(COL_RSSI_OFF), 0);
        lv_obj_set_style_bg_opa(_rssiBar[b], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(_rssiBar[b], 0, 0);
        lv_obj_set_style_radius(_rssiBar[b], 1, 0);
        lv_obj_set_style_pad_all(_rssiBar[b], 0, 0);
        lv_obj_clear_flag(_rssiBar[b], LV_OBJ_FLAG_SCROLLABLE);
    }

    _lblRssiVal = lv_label_create(info);
    lv_label_set_text(_lblRssiVal, "");
    lv_obj_set_style_text_color(_lblRssiVal, lv_color_hex(COL_TEXT_DIM), 0);
    lv_obj_set_style_text_font(_lblRssiVal, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblRssiVal, 170, (INFO_H - 12) / 2);

    _lblPps = lv_label_create(info);
    lv_label_set_text(_lblPps, "");
    lv_obj_set_style_text_color(_lblPps, lv_color_hex(COL_TEXT_DIM), 0);
    lv_obj_set_style_text_font(_lblPps, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(_lblPps, 220, (INFO_H - 12) / 2);

    _lblUptime = lv_label_create(info);
    lv_label_set_text(_lblUptime, "");
    lv_obj_set_style_text_color(_lblUptime, lv_color_hex(COL_TEXT_DIM), 0);
    lv_obj_set_style_text_font(_lblUptime, &lv_font_montserrat_12, 0);
    lv_label_set_long_mode(_lblUptime, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(_lblUptime, 64);
    lv_obj_set_style_text_align(_lblUptime, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(_lblUptime, LV_ALIGN_RIGHT_MID, -4, 0);

    /* Function rows (2-column) */
    for (uint8_t i = 0; i < 8; ++i) {
        int16_t col  = i % 2;
        int16_t row  = i / 2;
        int16_t rowY = INFO_H + row * CH_H;
        int16_t colX = col * COL_W;

        _chRowBg[i] = lv_obj_create(_controlPanel);
        lv_obj_set_size(_chRowBg[i], COL_W, CH_H);
        lv_obj_set_pos(_chRowBg[i], colX, rowY);
        uint32_t rowCol = (row % 2 == 0) ? COL_BG : COL_PANEL_BG;
        lv_obj_set_style_bg_color(_chRowBg[i], lv_color_hex(rowCol), 0);
        lv_obj_set_style_bg_opa(_chRowBg[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(_chRowBg[i], 0, 0);
        lv_obj_set_style_radius(_chRowBg[i], 0, 0);
        lv_obj_set_style_pad_all(_chRowBg[i], 0, 0);
        lv_obj_clear_flag(_chRowBg[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(_chRowBg[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(_chRowBg[i], funcRowClickCb, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(i)));

        _chNameLabel[i] = lv_label_create(_chRowBg[i]);
        lv_label_set_text(_chNameLabel[i], "---");
        lv_label_set_long_mode(_chNameLabel[i], LV_LABEL_LONG_CLIP);
        lv_obj_set_width(_chNameLabel[i], COL_NAME_W);
        lv_obj_set_style_text_color(_chNameLabel[i], lv_color_hex(COL_TEXT_DIM), 0);
        lv_obj_set_style_text_font(_chNameLabel[i], &lv_font_montserrat_12, 0);
        lv_obj_set_pos(_chNameLabel[i], 4, (CH_H - 12) / 2);

        _chBar[i] = lv_bar_create(_chRowBg[i]);
        lv_obj_set_size(_chBar[i], COL_BAR_W, CH_BAR_H);
        lv_obj_set_pos(_chBar[i], COL_BAR_X, (CH_H - CH_BAR_H) / 2);
        lv_bar_set_range(_chBar[i], static_cast<int32_t>(kChannelMin), static_cast<int32_t>(kChannelMax));
        lv_bar_set_value(_chBar[i], kChannelMid, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(_chBar[i], lv_color_hex(COL_BAR_BG), 0);
        lv_obj_set_style_radius(_chBar[i], 3, 0);
        lv_obj_set_style_bg_color(_chBar[i], lv_color_hex(COL_BAR_FG), LV_PART_INDICATOR);
        lv_obj_set_style_radius(_chBar[i], 3, LV_PART_INDICATOR);

        _chValLabel[i] = lv_label_create(_chRowBg[i]);
        lv_label_set_text(_chValLabel[i], "1500");
        lv_obj_set_style_text_color(_chValLabel[i], lv_color_hex(COL_TEXT), 0);
        lv_obj_set_style_text_font(_chValLabel[i], &lv_font_montserrat_12, 0);
        lv_obj_align(_chValLabel[i], LV_ALIGN_LEFT_MID, COL_BAR_X + COL_BAR_W / 2 - 14, 0);
    }

    /* Disconnect button bar */
    int16_t discY    = INFO_H + FUNC_ROWS * CH_H;
    lv_obj_t *btnBar = makePanel(_controlPanel, SCR_W, DISC_BAR_H, 0, discY, COL_PANEL_BG);

    _btnDisconnect = lv_btn_create(btnBar);
    lv_obj_set_size(_btnDisconnect, 120, DISC_BAR_H - 6);
    lv_obj_align(_btnDisconnect, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(_btnDisconnect, lv_color_hex(COL_BTN_RED), 0);
    lv_obj_set_style_radius(_btnDisconnect, 4, 0);
    lv_obj_add_event_cb(_btnDisconnect, disconnectBtnCb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *lblDisc = lv_label_create(_btnDisconnect);
    lv_label_set_text(lblDisc, "Disconnect");
    lv_obj_set_style_text_color(lblDisc, lv_color_hex(COL_TEXT), 0);
    lv_obj_set_style_text_font(lblDisc, &lv_font_montserrat_12, 0);
    lv_obj_center(lblDisc);

    /* Trim overlay panel */
    int16_t trimY = INFO_H + FUNC_ROWS * CH_H + DISC_BAR_H;
    _trimPanel    = makePanel(_controlPanel, SCR_W, TRIM_H, 0, trimY, COL_TRIM_BG);
    lv_obj_set_style_border_width(_trimPanel, 1, 0);
    lv_obj_set_style_border_color(_trimPanel, lv_color_hex(COL_TEXT_DIM), 0);
    lv_obj_set_style_border_side(_trimPanel, LV_BORDER_SIDE_TOP, 0);

    _lblTrimFunc = lv_label_create(_trimPanel);
    lv_label_set_text(_lblTrimFunc, "Trim: ---");
    lv_obj_set_style_text_color(_lblTrimFunc, lv_color_hex(COL_TEXT), 0);
    lv_obj_set_style_text_font(_lblTrimFunc, &lv_font_montserrat_12, 0);
    lv_obj_align(_lblTrimFunc, LV_ALIGN_LEFT_MID, 8, 0);

    _lblTrimVal = lv_label_create(_trimPanel);
    lv_label_set_text(_lblTrimVal, "0");
    lv_obj_set_style_text_color(_lblTrimVal, lv_color_hex(COL_TEXT), 0);
    lv_obj_set_style_text_font(_lblTrimVal, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(_lblTrimVal, 120, (TRIM_H - 14) / 2);

    _btnTrimUp = lv_btn_create(_trimPanel);
    lv_obj_set_size(_btnTrimUp, 40, TRIM_H - 8);
    lv_obj_align(_btnTrimUp, LV_ALIGN_RIGHT_MID, -100, 0);
    lv_obj_set_style_bg_color(_btnTrimUp, lv_color_hex(COL_BTN_BG), 0);
    lv_obj_set_style_radius(_btnTrimUp, 4, 0);
    lv_obj_add_event_cb(_btnTrimUp, trimUpBtnCb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *lblUp = lv_label_create(_btnTrimUp);
    lv_label_set_text(lblUp, LV_SYMBOL_UP);
    lv_obj_set_style_text_color(lblUp, lv_color_hex(COL_TEXT), 0);
    lv_obj_center(lblUp);

    _btnTrimDown = lv_btn_create(_trimPanel);
    lv_obj_set_size(_btnTrimDown, 40, TRIM_H - 8);
    lv_obj_align(_btnTrimDown, LV_ALIGN_RIGHT_MID, -54, 0);
    lv_obj_set_style_bg_color(_btnTrimDown, lv_color_hex(COL_BTN_BG), 0);
    lv_obj_set_style_radius(_btnTrimDown, 4, 0);
    lv_obj_add_event_cb(_btnTrimDown, trimDownBtnCb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *lblDn = lv_label_create(_btnTrimDown);
    lv_label_set_text(lblDn, LV_SYMBOL_DOWN);
    lv_obj_set_style_text_color(lblDn, lv_color_hex(COL_TEXT), 0);
    lv_obj_center(lblDn);

    _btnTrimClose = lv_btn_create(_trimPanel);
    lv_obj_set_size(_btnTrimClose, 36, TRIM_H - 8);
    lv_obj_align(_btnTrimClose, LV_ALIGN_RIGHT_MID, -8, 0);
    lv_obj_set_style_bg_color(_btnTrimClose, lv_color_hex(COL_BTN_RED), 0);
    lv_obj_set_style_radius(_btnTrimClose, 4, 0);
    lv_obj_add_event_cb(_btnTrimClose, trimCloseBtnCb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *lblCl = lv_label_create(_btnTrimClose);
    lv_label_set_text(lblCl, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(lblCl, lv_color_hex(COL_TEXT), 0);
    lv_obj_center(lblCl);

    lv_obj_add_flag(_trimPanel, LV_OBJ_FLAG_HIDDEN);

    /* Scan panel */
    _scanPanel = lv_obj_create(scr);
    lv_obj_set_size(_scanPanel, SCR_W, SCR_H - HDR_H);
    lv_obj_set_pos(_scanPanel, 0, HDR_H);
    lv_obj_set_style_bg_color(_scanPanel, lv_color_hex(COL_BG), 0);
    lv_obj_set_style_bg_opa(_scanPanel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_scanPanel, 0, 0);
    lv_obj_set_style_radius(_scanPanel, 0, 0);
    lv_obj_set_style_pad_all(_scanPanel, 0, 0);
    lv_obj_set_scroll_dir(_scanPanel, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_scanPanel, LV_SCROLLBAR_MODE_AUTO);

    for (uint8_t i = 0; i < kMaxDiscovered; ++i) {
        int16_t cardY = static_cast<int16_t>(i) * (SCAN_CARD_H + SCAN_CARD_GAP) + 6;

        _scanCards[i] = lv_obj_create(_scanPanel);
        lv_obj_set_size(_scanCards[i], SCAN_CARD_W, SCAN_CARD_H);
        lv_obj_set_pos(_scanCards[i], 12, cardY);
        lv_obj_set_style_bg_color(_scanCards[i], lv_color_hex(COL_CARD_BG), 0);
        lv_obj_set_style_bg_opa(_scanCards[i], LV_OPA_COVER, 0);
        lv_obj_set_style_radius(_scanCards[i], 8, 0);
        lv_obj_set_style_border_width(_scanCards[i], 0, 0);
        lv_obj_set_style_shadow_width(_scanCards[i], 8, 0);
        lv_obj_set_style_shadow_color(_scanCards[i], lv_color_hex(0x000000), 0);
        lv_obj_set_style_shadow_opa(_scanCards[i], LV_OPA_40, 0);
        lv_obj_set_style_pad_all(_scanCards[i], 0, 0);
        lv_obj_clear_flag(_scanCards[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(_scanCards[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(_scanCards[i], vehicleBtnCb, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(i)));
        lv_obj_add_flag(_scanCards[i], LV_OBJ_FLAG_HIDDEN);

        _scanModelIcon[i] = lv_obj_create(_scanCards[i]);
        lv_obj_set_size(_scanModelIcon[i], SCAN_ICON_SZ, SCAN_ICON_SZ);
        lv_obj_set_pos(_scanModelIcon[i], 8, (SCAN_CARD_H - SCAN_ICON_SZ) / 2);
        lv_obj_set_style_bg_color(_scanModelIcon[i], lv_color_hex(COL_MODEL_GENERIC), 0);
        lv_obj_set_style_bg_opa(_scanModelIcon[i], LV_OPA_COVER, 0);
        lv_obj_set_style_radius(_scanModelIcon[i], SCAN_ICON_SZ / 2, 0);
        lv_obj_set_style_border_width(_scanModelIcon[i], 0, 0);
        lv_obj_set_style_pad_all(_scanModelIcon[i], 0, 0);
        lv_obj_clear_flag(_scanModelIcon[i], LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *iconLbl = lv_label_create(_scanModelIcon[i]);
        lv_label_set_text(iconLbl, "G");
        lv_obj_set_style_text_color(iconLbl, lv_color_hex(COL_TEXT), 0);
        lv_obj_set_style_text_font(iconLbl, &lv_font_montserrat_14, 0);
        lv_obj_center(iconLbl);

        _scanModelLabel[i] = lv_label_create(_scanCards[i]);
        lv_label_set_text(_scanModelLabel[i], "");
        lv_obj_set_style_text_color(_scanModelLabel[i], lv_color_hex(COL_TEXT), 0);
        lv_obj_set_style_text_font(_scanModelLabel[i], &lv_font_montserrat_14, 0);
        lv_label_set_long_mode(_scanModelLabel[i], LV_LABEL_LONG_CLIP);
        lv_obj_set_width(_scanModelLabel[i], SCAN_CARD_W - SCAN_ICON_SZ - 60);
        lv_obj_set_pos(_scanModelLabel[i], SCAN_ICON_SZ + 16, 4);

        _scanNameLabel[i] = lv_label_create(_scanCards[i]);
        lv_label_set_text(_scanNameLabel[i], "");
        lv_obj_set_style_text_color(_scanNameLabel[i], lv_color_hex(COL_TEXT_DIM), 0);
        lv_obj_set_style_text_font(_scanNameLabel[i], &lv_font_montserrat_12, 0);
        lv_label_set_long_mode(_scanNameLabel[i], LV_LABEL_LONG_CLIP);
        lv_obj_set_width(_scanNameLabel[i], SCAN_CARD_W - SCAN_ICON_SZ - 60);
        lv_obj_set_pos(_scanNameLabel[i], SCAN_ICON_SZ + 16, 24);

        static const int16_t scanRssiH[4] = {4, 8, 12, 16};
        for (uint8_t b = 0; b < 4; ++b) {
            _scanRssiBars[i][b] = lv_obj_create(_scanCards[i]);
            int16_t bx          = SCAN_CARD_W - 40 + b * 7;
            int16_t bh          = scanRssiH[b];
            lv_obj_set_size(_scanRssiBars[i][b], 5, bh);
            lv_obj_set_pos(_scanRssiBars[i][b], bx, SCAN_CARD_H - bh - 6);
            lv_obj_set_style_bg_color(_scanRssiBars[i][b], lv_color_hex(COL_RSSI_OFF), 0);
            lv_obj_set_style_bg_opa(_scanRssiBars[i][b], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(_scanRssiBars[i][b], 0, 0);
            lv_obj_set_style_radius(_scanRssiBars[i][b], 1, 0);
            lv_obj_set_style_pad_all(_scanRssiBars[i][b], 0, 0);
            lv_obj_clear_flag(_scanRssiBars[i][b], LV_OBJ_FLAG_SCROLLABLE);
        }
    }

    _lblScanHint = lv_label_create(_scanPanel);
    lv_label_set_text(_lblScanHint, "Searching for vehicles...");
    lv_obj_set_style_text_color(_lblScanHint, lv_color_hex(COL_TEXT_DIM), 0);
    lv_obj_set_style_text_font(_lblScanHint, &lv_font_montserrat_12, 0);
    lv_obj_align(_lblScanHint, LV_ALIGN_BOTTOM_MID, 0, -6);

    showScanMode();
}

/* ── Mode switching ──────────────────────────────────────────────────────── */

void Display::showScanMode() {
    _scanMode = true;
    lv_obj_clear_flag(_scanPanel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_controlPanel, LV_OBJ_FLAG_HIDDEN);
}

void Display::showControlMode() {
    _scanMode = false;
    lv_obj_clear_flag(_controlPanel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_scanPanel, LV_OBJ_FLAG_HIDDEN);
}

/* ── refreshScan() ───────────────────────────────────────────────────────── */

void Display::refreshScan(const DiscoveredVehicle *vehicles, uint8_t count, uint8_t /*selectedIdx*/) {
    if (!_ready)
        return;
    if (!_scanMode)
        showScanMode();

    lv_label_set_text(_lblLinkBadge, LV_SYMBOL_EYE_OPEN " SCANNING");
    lv_obj_set_style_text_color(_lblLinkBadge, lv_color_hex(COL_BINDING), 0);

    for (uint8_t i = 0; i < kMaxDiscovered; ++i) {
        if (i < count && vehicles[i].valid) {
            lv_obj_clear_flag(_scanCards[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_bg_color(_scanModelIcon[i], lv_color_hex(modelIconColor(vehicles[i].modelType)), 0);
            lv_obj_t *iconLbl = lv_obj_get_child(_scanModelIcon[i], 0);
            if (iconLbl)
                lv_label_set_text(iconLbl, modelIconLetter(vehicles[i].modelType));

            lv_label_set_text(_scanModelLabel[i], modelName(static_cast<ModelType>(vehicles[i].modelType)).data());
            lv_label_set_text(_scanNameLabel[i], vehicles[i].name);

            uint8_t bars = rssiToBars(vehicles[i].rssi);
            for (uint8_t b = 0; b < 4; ++b) {
                uint32_t col = (b < bars) ? COL_RSSI_ON : COL_RSSI_OFF;
                lv_obj_set_style_bg_color(_scanRssiBars[i][b], lv_color_hex(col), 0);
            }
        } else {
            lv_obj_add_flag(_scanCards[i], LV_OBJ_FLAG_HIDDEN);
        }
    }

    lv_label_set_text(_lblScanHint, count == 0 ? "Searching for vehicles..." : "Tap a vehicle to connect");
    lv_task_handler();
}

/* ── refresh() ───────────────────────────────────────────────────────────── */

void Display::refresh(const TelemetryData &telemetry, const BatteryMonitor &txBattery, const ModuleManager & /*moduleManager*/, const FunctionValue *functions, uint8_t functionCount) {
    if (!_ready)
        return;
    if (_scanMode)
        showControlMode();

    /* Link badge */
    uint32_t badgeCol;
    const char *badgeText;
    switch (telemetry.linkState()) {
    case LinkState::Connected:
        badgeText = LV_SYMBOL_OK " CONNECTED";
        badgeCol  = COL_CONNECTED;
        break;
    case LinkState::Binding:
        badgeText = LV_SYMBOL_REFRESH " BINDING";
        badgeCol  = COL_BINDING;
        break;
    case LinkState::Failsafe:
        badgeText = LV_SYMBOL_WARNING " FAILSAFE";
        badgeCol  = COL_FAILSAFE;
        break;
    default:
        badgeText = "---";
        badgeCol  = COL_DISC;
        break;
    }
    lv_label_set_text(_lblLinkBadge, badgeText);
    lv_obj_set_style_text_color(_lblLinkBadge, lv_color_hex(badgeCol), 0);

    /* TX battery */
    _battWarnBlink = !_battWarnBlink;
    {
        char buf[20];
        uint16_t txCell = txBattery.cellVoltageMv();
        if (_battShowPercent && txCell > 0) {
            snprintf(buf, sizeof(buf), "TX:%3u%%", battCellPercent(txCell));
        } else {
            snprintf(buf, sizeof(buf), "TX:%.1fV", static_cast<double>(txBattery.voltageMv()) / 1000.0);
        }
        lv_label_set_text(_lblTxBatt, buf);
        uint32_t txCol = battCellColor(txCell);
        if (txCell > 0 && txCell <= 3300 && _battWarnBlink)
            lv_obj_set_style_text_color(_lblTxBatt, lv_color_hex(COL_BG), 0);
        else
            lv_obj_set_style_text_color(_lblTxBatt, lv_color_hex(txCol), 0);
    }

    /* RX battery */
    {
        char buf[20];
        if (telemetry.hasData()) {
            uint16_t rxCell = telemetry.rxCellVoltageMv();
            if (_battShowPercent && rxCell > 0)
                snprintf(buf, sizeof(buf), "RX:%3u%%", battCellPercent(rxCell));
            else
                snprintf(buf, sizeof(buf), "RX:%.1fV", static_cast<double>(telemetry.rxBatteryMv()) / 1000.0);
            uint32_t rxCol = battCellColor(rxCell);
            if (rxCell > 0 && rxCell <= 3300 && _battWarnBlink)
                lv_obj_set_style_text_color(_lblRxBatt, lv_color_hex(COL_BG), 0);
            else
                lv_obj_set_style_text_color(_lblRxBatt, lv_color_hex(rxCol), 0);
        } else {
            snprintf(buf, sizeof(buf), "RX: ---");
            lv_obj_set_style_text_color(_lblRxBatt, lv_color_hex(COL_TEXT_DIM), 0);
        }
        lv_label_set_text(_lblRxBatt, buf);
    }

    /* RSSI bars + numeric */
    {
        uint8_t bars = 0;
        if (telemetry.hasData()) {
            bars = rssiToBars(telemetry.rssi());
            char rbuf[12];
            snprintf(rbuf, sizeof(rbuf), "%ddBm", static_cast<int>(telemetry.rssi()));
            lv_label_set_text(_lblRssiVal, rbuf);
        } else {
            lv_label_set_text(_lblRssiVal, "");
        }
        for (uint8_t b = 0; b < 4; ++b) {
            uint32_t col = (b < bars) ? COL_RSSI_ON : COL_RSSI_OFF;
            lv_obj_set_style_bg_color(_rssiBar[b], lv_color_hex(col), 0);
        }
    }

    /* PPS */
    {
        char buf[12];
        snprintf(buf, sizeof(buf), "%.0f p/s", static_cast<double>(telemetry.packetsPerSecond()));
        lv_label_set_text(_lblPps, buf);
    }

    /* Uptime */
    {
        char buf[12];
        uint32_t totalSec = telemetry.connectionUptimeMs() / 1000u;
        uint32_t m        = totalSec / 60u;
        uint32_t s        = totalSec % 60u;
        snprintf(buf, sizeof(buf), "%lu:%02lu", static_cast<unsigned long>(m), static_cast<unsigned long>(s));
        lv_label_set_text(_lblUptime, buf);
    }

    /* Function bars */
    uint8_t drawCount = functionCount < 8u ? functionCount : 8u;
    for (uint8_t i = 0; i < drawCount; ++i) {
        uint16_t v = functions[i].value;
        lv_label_set_text(_chNameLabel[i], functionName(static_cast<Function>(functions[i].function)).data());
        lv_bar_set_value(_chBar[i], static_cast<int32_t>(v), LV_ANIM_OFF);

        int32_t pm        = barPermille(v);
        bool nearMid      = (pm >= 450 && pm <= 550);
        uint32_t barFgCol = nearMid ? COL_BAR_MID : COL_BAR_FG;
        lv_obj_set_style_bg_color(_chBar[i], lv_color_hex(barFgCol), LV_PART_INDICATOR);

        char buf[8];
        snprintf(buf, sizeof(buf), "%4d", static_cast<int>(v));
        lv_label_set_text(_chValLabel[i], buf);
    }

    for (uint8_t i = drawCount; i < 8; ++i)
        lv_obj_add_flag(_chRowBg[i], LV_OBJ_FLAG_HIDDEN);
    for (uint8_t i = 0; i < drawCount; ++i)
        lv_obj_clear_flag(_chRowBg[i], LV_OBJ_FLAG_HIDDEN);

    /* Trim panel update */
    if (_selectedTrimIdx >= 0 && _selectedTrimIdx < static_cast<int8_t>(drawCount)) {
        uint8_t ti = static_cast<uint8_t>(_selectedTrimIdx);
        char tbuf[24];
        snprintf(tbuf, sizeof(tbuf), "Trim: %s", functionName(static_cast<Function>(functions[ti].function)).data());
        lv_label_set_text(_lblTrimFunc, tbuf);
        char vbuf[8];
        snprintf(vbuf, sizeof(vbuf), "%d", static_cast<int>(functions[ti].trim));
        lv_label_set_text(_lblTrimVal, vbuf);
    }

    lv_task_handler();
}

} // namespace odh
