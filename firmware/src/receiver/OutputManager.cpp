#include "OutputManager.h"

#include <algorithm>
#include <cstring>

namespace odh {

OutputManager::OutputManager() {
    _channelValues.fill(kChannelMid);
    _failsafeValues.fill(kChannelMid);
}

void OutputManager::begin() {
    for (auto &driver : _drivers) {
        if (driver) {
            driver->begin();
        }
    }
}

void OutputManager::setDriver(uint8_t channel, std::unique_ptr<IOutputDriver> driver) {
    if (channel < _drivers.size()) {
        _drivers[channel] = std::move(driver);
    }
}

void OutputManager::applyControl(const ControlPacket &pkt) {
    const uint8_t fc = (pkt.function_count < kMaxFunctions) ? pkt.function_count : kMaxFunctions;

    for (uint8_t i = 0; i < fc; i++) {
        const auto func = static_cast<Function>(pkt.functions[i].function);
        const auto ch   = functionToChannel(_functionMap, func);

        if (!ch.has_value() || *ch >= config::rx::kChannelCount) {
            continue; // Function not mapped.
        }

        // Apply trim and clamp.
        int32_t val = static_cast<int32_t>(pkt.functions[i].value) + pkt.functions[i].trim;
        if (val < kChannelMin)
            val = kChannelMin;
        if (val > kChannelMax)
            val = kChannelMax;

        const auto us       = static_cast<uint16_t>(val);
        _channelValues[*ch] = us;

        if (_drivers[*ch]) {
            _drivers[*ch]->setChannel(us);
        }
    }
}

void OutputManager::applyFailsafe() {
    for (uint8_t i = 0; i < config::rx::kChannelCount; i++) {
        _channelValues[i] = _failsafeValues[i];
        if (_drivers[i]) {
            _drivers[i]->applyFailsafe();
        }
    }
}

void OutputManager::setChannel(uint8_t channel, uint16_t us) {
    if (channel >= config::rx::kChannelCount)
        return;

    if (us < kChannelMin)
        us = kChannelMin;
    if (us > kChannelMax)
        us = kChannelMax;

    _channelValues[channel] = us;
    if (_drivers[channel]) {
        _drivers[channel]->setChannel(us);
    }
}

void OutputManager::loadFromNvs() {
    NvsStore store("odh", true);

    _modelType = static_cast<ModelType>(store.getU8("model_type", static_cast<uint8_t>(ModelType::Generic)));

    const uint8_t count = store.getU8("fmap_count", 0);
    if (count > 0 && count <= kMaxFunctions) {
        store.getBytes("fmap_entries", _functionMap.entries.data(), count * sizeof(FunctionMapEntry));
        _functionMap.count = count;
    } else {
        _functionMap = defaultFunctionMap(_modelType);
    }

    // Load failsafe values.
    const size_t len = store.getBytes("fs_values", _failsafeValues.data(), sizeof(_failsafeValues));
    if (len != sizeof(_failsafeValues)) {
        _failsafeValues.fill(config::kFailsafeChannelValue);
    }

    // Clamp and push to drivers.
    for (uint8_t i = 0; i < config::rx::kChannelCount; i++) {
        if (_failsafeValues[i] < kChannelMin)
            _failsafeValues[i] = kChannelMin;
        if (_failsafeValues[i] > kChannelMax)
            _failsafeValues[i] = kChannelMax;
        if (_drivers[i]) {
            _drivers[i]->setFailsafeValue(_failsafeValues[i]);
        }
    }
}

void OutputManager::saveToNvs() {
    NvsStore store("odh", false);
    store.putU8("model_type", static_cast<uint8_t>(_modelType));
    store.putU8("fmap_count", _functionMap.count);
    store.putBytes("fmap_entries", _functionMap.entries.data(), _functionMap.count * sizeof(FunctionMapEntry));
    store.putBytes("fs_values", _failsafeValues.data(), sizeof(_failsafeValues));
}

void OutputManager::setModelType(ModelType model) {
    _modelType   = model;
    _functionMap = defaultFunctionMap(model);
}

uint16_t OutputManager::failsafeValue(uint8_t channel) const {
    if (channel >= config::rx::kChannelCount)
        return kChannelMid;
    return _failsafeValues[channel];
}

void OutputManager::setFailsafeValue(uint8_t channel, uint16_t us) {
    if (channel >= config::rx::kChannelCount)
        return;
    if (us < kChannelMin)
        us = kChannelMin;
    if (us > kChannelMax)
        us = kChannelMax;
    _failsafeValues[channel] = us;
    if (_drivers[channel]) {
        _drivers[channel]->setFailsafeValue(us);
    }
}

} // namespace odh
