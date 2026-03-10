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
