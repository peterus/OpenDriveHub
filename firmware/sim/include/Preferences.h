/**
 * Preferences.h – Simulation shim (in-memory key-value store).
 *
 * Values are stored in a std::map and are lost when the process exits.
 */

#ifndef SIM_PREFERENCES_H
#define SIM_PREFERENCES_H

#include "Arduino.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

class Preferences {
public:
    Preferences();
    ~Preferences();

    bool begin(const char *ns, bool readOnly = false);
    void end();

    uint8_t getUChar(const char *key, uint8_t defaultValue = 0) const;
    bool putUChar(const char *key, uint8_t value);

    uint16_t getUShort(const char *key, uint16_t defaultValue = 0) const;
    bool putUShort(const char *key, uint16_t value);

    int8_t getChar(const char *key, int8_t defaultValue = 0) const;
    bool putChar(const char *key, int8_t value);

    uint32_t getULong(const char *key, uint32_t defaultValue = 0) const;
    bool putULong(const char *key, uint32_t value);

    String getString(const char *key, const char *defaultValue = "") const;
    bool putString(const char *key, const char *value);

    size_t getBytes(const char *key, void *buf, size_t maxLen) const;
    size_t putBytes(const char *key, const void *value, size_t len);

    bool clear();

private:
    std::string _ns;

    using StoreMap = std::map<std::string, std::vector<uint8_t>>;
    static std::map<std::string, StoreMap> _store;

    std::string _fullKey(const char *key) const;
    StoreMap &_nsStore() const;
};

#endif /* SIM_PREFERENCES_H */
