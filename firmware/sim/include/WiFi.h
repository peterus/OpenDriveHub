/**
 * WiFi.h – Simulation shim (stubs).
 */

#ifndef SIM_WIFI_H
#define SIM_WIFI_H

#include <cstdint>

#include "Arduino.h"

typedef enum {
    WIFI_STA,
    WIFI_AP,
    WIFI_AP_STA,
    WIFI_OFF,
} wifi_mode_t;

class IPAddress {
    uint32_t _addr;
public:
    IPAddress() : _addr(0x0100007F) {} /* 127.0.0.1 */
    /* Implicit conversion lets Serial.print(ip) use the const char* overload. */
    operator const char *() const { return "127.0.0.1"; }
};

class WiFiClass {
public:
    void mode(wifi_mode_t) {}
    void disconnect(bool = false) {}
    bool softAP(const char *, const char * = nullptr) { return true; }
    void softAPdisconnect(bool = false) {}
    IPAddress softAPIP() { return IPAddress(); }
};

extern WiFiClass WiFi;

#endif /* SIM_WIFI_H */
