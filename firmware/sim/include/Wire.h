/**
 * Wire.h – Simulation shim (I²C stub).
 *
 * All I²C transactions return NACK (no device), so module detection will
 * find no hardware modules.  This is expected for the simulation.
 */

#ifndef SIM_WIRE_H
#define SIM_WIRE_H

#include <cstdint>
#include <cstddef>

class TwoWire {
public:
    void begin(int = -1, int = -1) {}
    void setClock(uint32_t) {}

    void beginTransmission(uint8_t) {}
    /** Always returns 2 (NACK) – no I²C devices present. */
    uint8_t endTransmission(bool = true) { return 2; }

    uint8_t requestFrom(uint8_t, uint8_t) { return 0; }
    size_t write(uint8_t) { return 1; }
    size_t write(const uint8_t *, size_t len) { return len; }
    int available() { return 0; }
    int read() { return -1; }
};

extern TwoWire Wire;

#endif /* SIM_WIRE_H */
