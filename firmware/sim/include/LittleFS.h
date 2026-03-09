/**
 * LittleFS simulation shim.
 * Provides a stub FS object that compiles but does nothing.
 */

#pragma once

class LittleFSFS {
public:
    bool begin(bool formatOnFail = false) {
        (void)formatOnFail;
        return true;
    }
    void end() {}
};

static LittleFSFS LittleFS;
