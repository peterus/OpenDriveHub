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
 * NvsStore – RAII wrapper around ESP32 Preferences for NVS access.
 *
 * Provides typed get/put helpers with automatic begin()/end() lifecycle.
 * Thread-safe when each instance is used from a single thread, or when
 * external locking is provided.
 */

#pragma once

#include <Preferences.h>

#include <cstdint>
#include <cstring>

namespace odh {

/**
 * RAII wrapper for the Arduino-ESP32 Preferences library.
 *
 * Usage:
 *   {
 *       NvsStore store("odh");
 *       auto model = store.getU8("model_type", 0);
 *       store.putU8("model_type", 2);
 *   }  // store.end() called automatically
 */
class NvsStore {
public:
    /**
     * Open an NVS namespace.
     * @param ns         Namespace name (max 15 chars).
     * @param readOnly   true for read-only access.
     */
    explicit NvsStore(const char *ns, bool readOnly = false)
        : _open(false) {
        _open = _prefs.begin(ns, readOnly);
    }

    ~NvsStore() {
        if (_open) {
            _prefs.end();
        }
    }

    // Non-copyable, non-movable (Preferences is not movable).
    NvsStore(const NvsStore &)            = delete;
    NvsStore &operator=(const NvsStore &) = delete;
    NvsStore(NvsStore &&)                 = delete;
    NvsStore &operator=(NvsStore &&)      = delete;

    /// True if the NVS namespace was opened successfully.
    bool isOpen() const {
        return _open;
    }

    // ── Scalar types ────────────────────────────────────────────────────

    uint8_t getU8(const char *key, uint8_t def = 0) const {
        return _prefs.getUChar(key, def);
    }

    bool putU8(const char *key, uint8_t value) {
        return _prefs.putUChar(key, value);
    }

    uint16_t getU16(const char *key, uint16_t def = 0) const {
        return _prefs.getUShort(key, def);
    }

    bool putU16(const char *key, uint16_t value) {
        return _prefs.putUShort(key, value);
    }

    int8_t getI8(const char *key, int8_t def = 0) const {
        return _prefs.getChar(key, def);
    }

    bool putI8(const char *key, int8_t value) {
        return _prefs.putChar(key, value);
    }

    uint32_t getU32(const char *key, uint32_t def = 0) const {
        return _prefs.getULong(key, def);
    }

    bool putU32(const char *key, uint32_t value) {
        return _prefs.putULong(key, value);
    }

    // ── String ──────────────────────────────────────────────────────────

    String getString(const char *key, const char *def = "") const {
        return _prefs.getString(key, def);
    }

    bool putString(const char *key, const char *value) {
        return _prefs.putString(key, value);
    }

    // ── Raw byte blobs ──────────────────────────────────────────────────

    size_t getBytes(const char *key, void *buf, size_t maxLen) const {
        return _prefs.getBytes(key, buf, maxLen);
    }

    size_t putBytes(const char *key, const void *data, size_t len) {
        return _prefs.putBytes(key, data, len);
    }

    /// Typed blob helper – read a POD struct or array.
    template <typename T> bool getBlob(const char *key, T &out) const {
        return getBytes(key, &out, sizeof(T)) == sizeof(T);
    }

    /// Typed blob helper – write a POD struct or array.
    template <typename T> bool putBlob(const char *key, const T &data) {
        return putBytes(key, &data, sizeof(T)) == sizeof(T);
    }

    /// Direct access to the underlying Preferences (escape hatch).
    Preferences &raw() {
        return _prefs;
    }
    const Preferences &raw() const {
        return _prefs;
    }

private:
    mutable Preferences _prefs; // mutable: ESP32 Preferences API is not const-correct
    bool _open;
};

} // namespace odh
