/**
 * sim_arduino.cpp – Arduino API implementation for Linux simulation.
 */

#include "Arduino.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>

/* ── Global instances ───────────────────────────────────────────────────── */

HardwareSerial Serial;

/* ── Time ───────────────────────────────────────────────────────────────── */

static auto s_startTime = std::chrono::steady_clock::now();

uint32_t millis() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - s_startTime).count());
}

uint32_t micros() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(now - s_startTime).count());
}

void delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void delayMicroseconds(uint32_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void yield() {
    std::this_thread::yield();
}

/* ── GPIO stubs ─────────────────────────────────────────────────────────── */

void pinMode(uint8_t, uint8_t) {}

void digitalWrite(uint8_t, uint8_t) {}

int digitalRead(uint8_t) {
    /* Return HIGH (pull-up default) – config button NOT pressed. */
    return HIGH;
}

int analogRead(uint8_t) {
    /* Return ~2.9V equivalent for a 12-bit ADC (simulates ~12V battery). */
    return 3600;
}

/* ── MAC address ────────────────────────────────────────────────────────── */

void esp_read_mac(uint8_t *mac, esp_mac_type_t) {
#ifdef SIM_TX
    static const uint8_t fakeMac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0x01, 0x01};
#else
    static const uint8_t fakeMac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0x02, 0x01};
#endif
    memcpy(mac, fakeMac, 6);
}

/* ── Arduino main() bridge ──────────────────────────────────────────────── */

int main(int, char **) {
    /* Line-buffer stdout so Serial.print/println output is visible immediately
     * even when piped through PlatformIO's exec runner. */
    setvbuf(stdout, nullptr, _IOLBF, 0);
    setup();
    for (;;) {
        loop();
    }
    return 0;
}
