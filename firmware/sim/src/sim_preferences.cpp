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
 * sim_preferences.cpp – In-memory NVS implementation.
 */

#include "Preferences.h"

#include <cstring>

/* ── Static store: namespace → (key → blob) ─────────────────────────────── */

std::map<std::string, Preferences::StoreMap> Preferences::_store;

Preferences::Preferences() {}
Preferences::~Preferences() {}

bool Preferences::begin(const char *ns, bool) {
    _ns = ns ? ns : "default";
    return true;
}

void Preferences::end() {}

std::string Preferences::_fullKey(const char *key) const {
    return _ns + "/" + key;
}

Preferences::StoreMap &Preferences::_nsStore() const {
    return _store[_ns];
}

uint8_t Preferences::getUChar(const char *key, uint8_t defaultValue) const {
    auto &store = _nsStore();
    auto it     = store.find(key);
    if (it == store.end() || it->second.size() < 1)
        return defaultValue;
    return it->second[0];
}

bool Preferences::putUChar(const char *key, uint8_t value) {
    _nsStore()[key] = {value};
    return true;
}

uint16_t Preferences::getUShort(const char *key, uint16_t defaultValue) const {
    auto &store = _nsStore();
    auto it     = store.find(key);
    if (it == store.end() || it->second.size() < 2)
        return defaultValue;
    uint16_t v;
    memcpy(&v, it->second.data(), sizeof(v));
    return v;
}

bool Preferences::putUShort(const char *key, uint16_t value) {
    auto *p         = reinterpret_cast<const uint8_t *>(&value);
    _nsStore()[key] = std::vector<uint8_t>(p, p + sizeof(value));
    return true;
}

int8_t Preferences::getChar(const char *key, int8_t defaultValue) const {
    auto &store = _nsStore();
    auto it     = store.find(key);
    if (it == store.end() || it->second.size() < 1)
        return defaultValue;
    return static_cast<int8_t>(it->second[0]);
}

bool Preferences::putChar(const char *key, int8_t value) {
    _nsStore()[key] = {static_cast<uint8_t>(value)};
    return true;
}

uint32_t Preferences::getULong(const char *key, uint32_t defaultValue) const {
    auto &store = _nsStore();
    auto it     = store.find(key);
    if (it == store.end() || it->second.size() < 4)
        return defaultValue;
    uint32_t v;
    memcpy(&v, it->second.data(), sizeof(v));
    return v;
}

bool Preferences::putULong(const char *key, uint32_t value) {
    auto *p         = reinterpret_cast<const uint8_t *>(&value);
    _nsStore()[key] = std::vector<uint8_t>(p, p + sizeof(value));
    return true;
}

String Preferences::getString(const char *key, const char *defaultValue) const {
    auto &store = _nsStore();
    auto it     = store.find(key);
    if (it == store.end())
        return String(defaultValue);
    return String(std::string(it->second.begin(), it->second.end()).c_str());
}

bool Preferences::putString(const char *key, const char *value) {
    std::string s(value);
    _nsStore()[key] = std::vector<uint8_t>(s.begin(), s.end());
    return true;
}

size_t Preferences::getBytes(const char *key, void *buf, size_t maxLen) const {
    auto &store = _nsStore();
    auto it     = store.find(key);
    if (it == store.end())
        return 0;
    size_t copyLen = it->second.size() < maxLen ? it->second.size() : maxLen;
    memcpy(buf, it->second.data(), copyLen);
    return copyLen;
}

size_t Preferences::putBytes(const char *key, const void *value, size_t len) {
    auto *p         = static_cast<const uint8_t *>(value);
    _nsStore()[key] = std::vector<uint8_t>(p, p + len);
    return len;
}

bool Preferences::clear() {
    _nsStore().clear();
    return true;
}
