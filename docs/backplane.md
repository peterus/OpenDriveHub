# Backplane

The OpenDriveHub backplane is an **I²C multiplexer bus** that connects
plug-in input modules to the transmitter.  It is built around a
**TCA9548A** 8-channel I²C switch.

---

## How It Works

The TCA9548A sits on the ESP32's main I²C bus (SDA = GPIO21, SCL = GPIO22)
at address **0x70** (configurable in `odh::config::tx::kI2cMuxAddr`).

Each module is connected to one of the mux's eight downstream channel pairs
(SD0/SC0 through SD7/SC7).  Because only one channel is active at a time,
modules on different slots may use the same I²C address without conflict.

```
ESP32 (SDA, SCL)
   │
   └── TCA9548A  (0x70)
         ├── CH0 → Slot 0 module (e.g. PotModule at 0x48)
         ├── CH1 → Slot 1 module (e.g. SwitchModule at 0x20)
         ├── CH2 → Slot 2 module …
         ├── CH3 → (empty)
         ├── CH4 → …
         ├── CH5 → …
         ├── CH6 → (not populated)
         └── CH7 → (not populated)
```

The default slot count is **6** (`odh::config::tx::kModuleSlotCount`).
This is a software limit; the hardware supports up to 8.

---

## Firmware Interface

The backplane is managed by the **`odh::Backplane`** class
(`firmware/src/transmitter/backplane/Backplane.h`).

### Key API

```cpp
namespace odh {

class Backplane {
public:
    explicit Backplane(uint8_t muxAddress = 0x70, uint8_t slotCount = 8);

    bool begin();                   // Initialise and verify TCA9548A
    bool selectSlot(uint8_t slot);  // Activate one channel
    void deselectAll();             // Deactivate all channels
    uint8_t activeSlot() const;     // Current active slot (kNoSlot if none)
    uint8_t scanSlots(uint8_t addr);// Probe each slot for a device
    uint8_t slotCount() const;
    bool isReady() const;
};

} // namespace odh
```

### Usage Pattern

```cpp
odh::Backplane bp(odh::config::tx::kI2cMuxAddr, odh::config::tx::kModuleSlotCount);
bp.begin();

for (uint8_t i = 0; i < bp.slotCount(); i++) {
    bp.selectSlot(i);
    // probe / read from whatever device is on this channel
}
bp.deselectAll();
```

---

## Module Detection

The **`ModuleManager`** uses `Backplane::selectSlot()` to iterate over each
slot and probes a fixed set of I²C addresses to identify the module type:

| Probe Address | Chip | Module Type | Class |
|---------------|------|-------------|-------|
| 0x48 | ADS1115 | `ModuleType::Potentiometer` | `PotModule` |
| 0x20 | PCF8574 | `ModuleType::Switch` | `SwitchModule` |
| 0x21 | PCF8574 | `ModuleType::Button` | `ButtonModule` |
| 0x36 | AS5600 | `ModuleType::Encoder` | `EncoderModule` |

If no known device responds at any of the probed addresses, the slot is
marked empty.  Detection runs once at startup.

The module type enum is defined in `firmware/src/transmitter/modules/IModule.h`:

```cpp
enum class ModuleType : uint8_t {
    Unknown       = 0x00,
    Switch        = 0x01,
    Button        = 0x02,
    Potentiometer = 0x03,
    Encoder       = 0x04,
};
```

---

## Adding a New Module Type

1. **Create the header and source** in `firmware/src/transmitter/modules/`:

   ```cpp
   // MyModule.h
   #pragma once
   #include "IModule.h"

   namespace odh {

   class MyModule : public IModule {
   public:
       explicit MyModule(uint8_t slot);
       bool begin() override;
       void update() override;
       uint8_t inputCount() const override;
       uint16_t inputValue(uint8_t index) const override;
   };

   } // namespace odh
   ```

2. **Add a new enum value** to `ModuleType`:

   ```cpp
   enum class ModuleType : uint8_t {
       // ... existing types ...
       MyDevice = 0x05,
   };
   ```

3. **Register the probe** in `ModuleManager` – add your I²C address and
   instantiation logic to the detection loop.

4. **Design the hardware module** – create a directory under
   `hardware/modules/your_module/` with a `README.md` describing:
   - Chip / breakout board used
   - I²C address (must not conflict with existing modules)
   - Schematic / wiring table
   - BOM

---

## Hardware Notes

### I²C Pull-ups

The TCA9548A requires pull-up resistors on the **upstream** bus (ESP32 side):
- 4.7 kΩ from 3V3 to SDA
- 4.7 kΩ from 3V3 to SCL

Module breakout boards (ADS1115, PCF8574, AS5600) typically include their
own pull-ups on the downstream side.  If using a bare chip, add 4.7 kΩ
pull-ups on each downstream channel pair as well.

### Clock Speed

The bus runs at **400 kHz** fast-mode (`odh::config::kI2cFreqHz`).  All
currently supported chips handle this speed.

### Power

All modules and the mux are powered from the transmitter's 3.3 V rail.
Current draw per slot is typically < 5 mA, but analog modules with
potentiometers connected to 3.3 V may draw more through the wiper.

---

## Slot Connector

See `hardware/transmitter/README.md` for the physical connector pinout.
Each slot header exposes:

| Pin | Signal |
|-----|--------|
| 1 | 3V3 |
| 2 | GND |
| 3 | SDAn (TCA9548A channel N) |
| 4 | SCLn (TCA9548A channel N) |
