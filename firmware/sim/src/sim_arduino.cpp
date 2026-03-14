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
 * sim_arduino.cpp – Arduino API implementation for Linux simulation.
 */

#include <esp_now.h>

#include "Arduino.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>

#ifdef __has_include
#if __has_include(<termios.h>)
#include <fcntl.h>
#include <termios.h>
#define HAS_TERMIOS 1
#endif
#endif

/* ── Global instances ───────────────────────────────────────────────────── */

HardwareSerial Serial;

/* ── HardwareSerial output ──────────────────────────────────────────────── */

void HardwareSerial::initOutput() {
    if (_outReady)
        return;
    _outReady = true;

    if (isatty(STDOUT_FILENO)) {
        _out = stdout;
        return;
    }

    // stdout is a pipe (e.g. PlatformIO exec).  PlatformIO's BuildAsyncPipe
    // buffers output line-by-line, which breaks interactive shell prompts
    // and character echo.  Write directly to the controlling terminal.
    _out = fopen("/dev/tty", "w");
    if (_out) {
        setvbuf(_out, nullptr, _IONBF, 0);
    } else {
        _out = stdout; // no terminal – fall back to pipe
    }
}

size_t HardwareSerial::print(const char *s) {
    return static_cast<size_t>(fprintf(outputStream(), "%s", s));
}
size_t HardwareSerial::print(int v) {
    return static_cast<size_t>(fprintf(outputStream(), "%d", v));
}
size_t HardwareSerial::print(unsigned v) {
    return static_cast<size_t>(fprintf(outputStream(), "%u", v));
}
size_t HardwareSerial::print(long v) {
    return static_cast<size_t>(fprintf(outputStream(), "%ld", v));
}
size_t HardwareSerial::print(unsigned long v) {
    return static_cast<size_t>(fprintf(outputStream(), "%lu", v));
}
size_t HardwareSerial::print(double v, int prec) {
    return static_cast<size_t>(fprintf(outputStream(), "%.*f", prec, v));
}

size_t HardwareSerial::println() {
    return static_cast<size_t>(fprintf(outputStream(), "\n"));
}
size_t HardwareSerial::println(const char *s) {
    return static_cast<size_t>(fprintf(outputStream(), "%s\n", s));
}
size_t HardwareSerial::println(int v) {
    return static_cast<size_t>(fprintf(outputStream(), "%d\n", v));
}
size_t HardwareSerial::println(unsigned v) {
    return static_cast<size_t>(fprintf(outputStream(), "%u\n", v));
}
size_t HardwareSerial::println(long v) {
    return static_cast<size_t>(fprintf(outputStream(), "%ld\n", v));
}
size_t HardwareSerial::println(unsigned long v) {
    return static_cast<size_t>(fprintf(outputStream(), "%lu\n", v));
}
size_t HardwareSerial::println(double v, int prec) {
    return static_cast<size_t>(fprintf(outputStream(), "%.*f\n", prec, v));
}

size_t HardwareSerial::print(const std::string &s) {
    return static_cast<size_t>(fprintf(outputStream(), "%s", s.c_str()));
}
size_t HardwareSerial::println(const std::string &s) {
    return static_cast<size_t>(fprintf(outputStream(), "%s\n", s.c_str()));
}

size_t HardwareSerial::printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = vfprintf(outputStream(), fmt, args);
    va_end(args);
    return r > 0 ? static_cast<size_t>(r) : 0;
}

/* ── Time ───────────────────────────────────────────────────────────────── */

static auto s_startTime = std::chrono::steady_clock::now();

uint32_t millis() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now - s_startTime).count());
}

uint32_t micros() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(now - s_startTime).count());
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

/// Generate a per-process unique MAC address so that multiple simulation
/// instances of the same role (e.g. two sim_tx) are distinguishable.
/// The last two bytes are derived from the process ID.
void esp_read_mac(uint8_t *mac, esp_mac_type_t) {
    static uint8_t fakeMac[6] = {};
    static bool initialised   = false;
    if (!initialised) {
        uint16_t pid = static_cast<uint16_t>(getpid());
#ifdef SIM_TX
        fakeMac[0] = 0xAA;
        fakeMac[1] = 0xBB;
        fakeMac[2] = 0xCC;
        fakeMac[3] = 0xDD;
#else
        fakeMac[0] = 0xAA;
        fakeMac[1] = 0xBB;
        fakeMac[2] = 0xCC;
        fakeMac[3] = 0xDE;
#endif
        fakeMac[4]  = static_cast<uint8_t>(pid >> 8);
        fakeMac[5]  = static_cast<uint8_t>(pid & 0xFF);
        initialised = true;
    }
    memcpy(mac, fakeMac, 6);
}

/* ── Arduino main() bridge ──────────────────────────────────────────────── */

#ifdef HAS_TERMIOS
static struct termios s_origTermios;
static int s_ttyFd = -1;

/// Enable raw mode on the given file descriptor.
static void enableRawModeOnFd(int fd) {
    tcgetattr(fd, &s_origTermios);
    atexit([] {
        int restoreFd = (s_ttyFd >= 0) ? s_ttyFd : STDIN_FILENO;
        tcsetattr(restoreFd, TCSAFLUSH, &s_origTermios);
    });

    struct termios raw = s_origTermios;
    raw.c_lflag &= ~(static_cast<tcflag_t>(ICANON) | static_cast<tcflag_t>(ECHO));
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &raw);
}

/// Open the controlling terminal for input.  When stdin is a pipe (e.g.
/// PlatformIO exec), /dev/tty gives us direct access to the real terminal.
/// Returns the fd to read from (STDIN_FILENO or /dev/tty).
static int openTerminalInput() {
    if (isatty(STDIN_FILENO)) {
        enableRawModeOnFd(STDIN_FILENO);
        return STDIN_FILENO;
    }
    // stdin is a pipe – try the controlling terminal directly.
    int fd = open("/dev/tty", O_RDONLY);
    if (fd >= 0) {
        s_ttyFd = fd;
        enableRawModeOnFd(fd);
        return fd;
    }
    // No terminal at all (CI, headless server) – fall back to stdin pipe.
    return STDIN_FILENO;
}
#endif

/// Background thread that reads from the terminal (or stdin) and feeds
/// bytes into the Serial receive ring buffer.
static void stdinReaderThread(int fd) {
    while (true) {
        uint8_t c;
        ssize_t n = ::read(fd, &c, 1);
        if (n <= 0)
            break;
        Serial.pushRxByte(c);
    }
}

/// Parse command-line arguments:
///   --rssi <ch>:<dBm>[,<ch>:<dBm>]   Set simulated RSSI per channel
static void parseArgs(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--rssi") == 0 && i + 1 < argc) {
            ++i;
            // Parse "ch:rssi" pairs, e.g. "6:-30" or "1:-45,6:-30,11:-70"
            char *arg = argv[i];
            while (arg && *arg) {
                int ch = 0, rssi = 0;
                if (std::sscanf(arg, "%d:%d", &ch, &rssi) == 2) {
                    sim_set_channel_rssi(static_cast<uint8_t>(ch), static_cast<int8_t>(rssi));
                    printf("[SIM] Channel %d RSSI set to %d dBm\n", ch, rssi);
                }
                // Advance to next comma-separated pair
                char *comma = std::strchr(arg, ',');
                arg         = comma ? comma + 1 : nullptr;
            }
        }
    }
}

int main(int argc, char **argv) {
    /* Disable buffering on both stdout and stdin so the interactive shell
     * prompt, character echo and input are immediate. */
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stdin, nullptr, _IONBF, 0);

    parseArgs(argc, argv);

    int inputFd = STDIN_FILENO;
#ifdef HAS_TERMIOS
    inputFd = openTerminalInput();
#endif

    /* Start stdin reader as a background daemon thread. */
    std::thread reader(stdinReaderThread, inputFd);
    reader.detach();

    setup();
    for (;;) {
        loop();
    }
    return 0;
}
