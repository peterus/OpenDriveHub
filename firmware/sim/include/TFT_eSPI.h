/**
 * TFT_eSPI.h – Simulation shim (stub).
 *
 * Under NATIVE_SIM, Display.cpp uses SDL2 directly instead of TFT_eSPI.
 * This stub provides the class interface so the code compiles, but the
 * methods are only called in the non-NATIVE_SIM code path.
 */

#ifndef SIM_TFT_ESPI_H
#define SIM_TFT_ESPI_H

#include <cstdint>

#define TFT_BLACK 0x0000

class TFT_eSPI {
public:
    void begin() {}
    void setRotation(uint8_t) {}
    void fillScreen(uint16_t) {}
    void setTouch(uint16_t *) {}
    bool getTouch(uint16_t *, uint16_t *, uint16_t = 600) { return false; }

    void startWrite() {}
    void endWrite() {}
    void setAddrWindow(int32_t, int32_t, uint32_t, uint32_t) {}
    void pushColors(uint16_t *, uint32_t, bool = true) {}
};

#endif /* SIM_TFT_ESPI_H */
