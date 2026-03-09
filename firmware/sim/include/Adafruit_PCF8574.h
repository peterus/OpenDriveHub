/**
 * Adafruit_PCF8574 simulation shim.
 * Provides a stub so transmitter code compiles on native/sim.
 */

#pragma once

#include <Wire.h>

#include <cstdint>

#ifndef INPUT
#define INPUT 0x0
#endif

class Adafruit_PCF8574 {
public:
    bool begin(uint8_t addr = 0x20, TwoWire *wire = nullptr) {
        (void)addr;
        (void)wire;
        return true;
    }
    void pinMode(uint8_t pin, uint8_t mode) {
        (void)pin;
        (void)mode;
    }
    bool digitalRead(uint8_t pin) {
        (void)pin;
        return true;
    }
    void digitalWrite(uint8_t pin, bool val) {
        (void)pin;
        (void)val;
    }
};
