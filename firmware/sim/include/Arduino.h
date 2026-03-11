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
 * Arduino.h – Simulation shim for native Linux builds.
 *
 * Provides the subset of the Arduino API used by the OpenDriveHub firmware:
 * millis(), delay(), Serial, GPIO stubs, ADC stubs, String class, and the
 * F() macro.  The Arduino main() calling convention (setup/loop) is also
 * provided so that the original firmware main.cpp compiles unchanged.
 */

#ifndef SIM_ARDUINO_H
#define SIM_ARDUINO_H

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>

/* ── Arduino-compatible types ───────────────────────────────────────────── */

typedef uint8_t byte;
typedef bool boolean;

/* ── Time ───────────────────────────────────────────────────────────────── */

uint32_t millis();
uint32_t micros();
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);
void yield();

/* ── GPIO ───────────────────────────────────────────────────────────────── */

#define INPUT 0x00
#define OUTPUT 0x01
#define INPUT_PULLUP 0x02

#define LOW 0
#define HIGH 1

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);

/* ── LEDC (PWM) stubs ──────────────────────────────────────────────────── */

inline void ledcSetup(uint8_t, uint32_t, uint8_t) {}
inline void ledcAttachPin(uint8_t, uint8_t) {}
inline void ledcWrite(uint8_t, uint32_t) {}

/* ── MAC address ────────────────────────────────────────────────────────── */

typedef enum {
    ESP_MAC_WIFI_STA,
    ESP_MAC_WIFI_SOFTAP,
    ESP_MAC_BT,
    ESP_MAC_ETH,
} esp_mac_type_t;

void esp_read_mac(uint8_t *mac, esp_mac_type_t type);

/* ── ESP error codes ────────────────────────────────────────────────────── */

typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL (-1)

/* ── Serial ─────────────────────────────────────────────────────────────── */

/**
 * Minimal Serial class that prints to stdout and (in simulation builds)
 * reads from a ring buffer fed by a background stdin-reader thread.
 *
 * When stdout is not a terminal (e.g. PlatformIO pipes it through an
 * async reader), output is written directly to /dev/tty so that shell
 * prompts and character echo are visible immediately.
 */
class HardwareSerial {
public:
    void begin(unsigned long) {
        initOutput();
    }
    void end() {}

    /// Initialise output – call before first print if begin() is skipped.
    void initOutput();

    /* ── Input (ring-buffer backed by stdin reader thread) ──────────── */

    int available() {
        std::lock_guard<std::mutex> lock(_rxMutex);
        return static_cast<int>((_rxHead - _rxTail + kRxBufSize) % kRxBufSize);
    }

    int read() {
        std::lock_guard<std::mutex> lock(_rxMutex);
        if (_rxHead == _rxTail)
            return -1;
        uint8_t c = _rxBuf[_rxTail];
        _rxTail   = (_rxTail + 1) % kRxBufSize;
        return c;
    }

    int peek() {
        std::lock_guard<std::mutex> lock(_rxMutex);
        if (_rxHead == _rxTail)
            return -1;
        return _rxBuf[_rxTail];
    }

    /// Push a byte into the receive buffer (called by the stdin reader thread).
    void pushRxByte(uint8_t c) {
        std::lock_guard<std::mutex> lock(_rxMutex);
        uint16_t next = (_rxHead + 1) % kRxBufSize;
        if (next != _rxTail) {
            _rxBuf[_rxHead] = c;
            _rxHead         = next;
        }
    }

    /* ── Output ────────────────────────────────────────────────────── */

    size_t print(const char *s);
    size_t print(int v);
    size_t print(unsigned v);
    size_t print(long v);
    size_t print(unsigned long v);
    size_t print(double v, int prec = 2);

    size_t println();
    size_t println(const char *s);
    size_t println(int v);
    size_t println(unsigned v);
    size_t println(long v);
    size_t println(unsigned long v);
    size_t println(double v, int prec = 2);

    size_t print(const std::string &s);
    size_t println(const std::string &s);

    size_t printf(const char *fmt, ...) __attribute__((format(printf, 2, 3)));

    /// Return the output FILE* stream, initialising on first call.
    FILE *outputStream() {
        if (!_outReady)
            initOutput();
        return _out;
    }

private:
    static constexpr uint16_t kRxBufSize = 256;
    uint8_t _rxBuf[kRxBufSize]           = {};
    uint16_t _rxHead                     = 0;
    uint16_t _rxTail                     = 0;
    std::mutex _rxMutex;

    FILE *_out     = nullptr; // output stream (stdout or /dev/tty)
    bool _outReady = false;
};

extern HardwareSerial Serial;

/* ── F() macro (no-op on native) ────────────────────────────────────────── */

#define F(string_literal) (string_literal)

/* ── Arduino String class (thin wrapper around std::string) ─────────────── */

class String : public std::string {
public:
    String()
        : std::string() {}
    String(const char *s)
        : std::string(s ? s : "") {}
    String(const std::string &s)
        : std::string(s) {}
    String(int val)
        : std::string(std::to_string(val)) {}

    int indexOf(const char *sub, int from = 0) const {
        auto pos = find(sub, from);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }
    int indexOf(char c, int from = 0) const {
        auto pos = find(c, from);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }

    String substring(int from) const {
        if (from < 0 || from >= static_cast<int>(size()))
            return String("");
        return String(std::string::substr(from));
    }
    String substring(int from, int to) const {
        if (from < 0)
            from = 0;
        if (to < from)
            return String("");
        return String(std::string::substr(from, to - from));
    }

    int toInt() const {
        try {
            return std::stoi(*this);
        } catch (...) {
            return 0;
        }
    }

    void toCharArray(char *buf, unsigned len) const {
        strncpy(buf, c_str(), len);
        if (len > 0)
            buf[len - 1] = '\0';
    }

    /* ArduinoJson serialization requires a write() method on String. */
    size_t write(uint8_t c) {
        push_back(static_cast<char>(c));
        return 1;
    }
    size_t write(const uint8_t *buf, size_t n) {
        append(reinterpret_cast<const char *>(buf), n);
        return n;
    }
};

/* ── Arduino setup/loop → main() bridge ─────────────────────────────────── */

void setup();
void loop();

#endif /* SIM_ARDUINO_H */
