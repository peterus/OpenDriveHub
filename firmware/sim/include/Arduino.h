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
 * Minimal Serial class that prints to stdout.
 */
class HardwareSerial {
public:
    void begin(unsigned long) {}
    void end() {}

    /* print / println for basic types */
    size_t print(const char *s) {
        return printf("%s", s);
    }
    size_t print(int v) {
        return printf("%d", v);
    }
    size_t print(unsigned v) {
        return printf("%u", v);
    }
    size_t print(long v) {
        return printf("%ld", v);
    }
    size_t print(unsigned long v) {
        return printf("%lu", v);
    }
    size_t print(double v, int prec = 2) {
        return printf("%.*f", prec, v);
    }

    size_t println() {
        return printf("\n");
    }
    size_t println(const char *s) {
        return printf("%s\n", s);
    }
    size_t println(int v) {
        return printf("%d\n", v);
    }
    size_t println(unsigned v) {
        return printf("%u\n", v);
    }
    size_t println(long v) {
        return printf("%ld\n", v);
    }
    size_t println(unsigned long v) {
        return printf("%lu\n", v);
    }
    size_t println(double v, int prec = 2) {
        return printf("%.*f\n", prec, v);
    }

    /* The F() macro just passes through the string on native. */
    size_t print(const std::string &s) {
        return printf("%s", s.c_str());
    }
    size_t println(const std::string &s) {
        return printf("%s\n", s.c_str());
    }

    /* ESP32-Arduino Serial.printf() */
    size_t printf(const char *fmt, ...) __attribute__((format(printf, 2, 3))) {
        va_list args;
        va_start(args, fmt);
        int r = vprintf(fmt, args);
        va_end(args);
        return r > 0 ? static_cast<size_t>(r) : 0;
    }
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
