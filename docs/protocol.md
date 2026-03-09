# Protocol Specification

This document describes the radio protocol used between the OpenDriveHub
transmitter and receiver.

**Source file:** `firmware/lib/odh-protocol/Protocol.h`
**Function map:** `firmware/lib/odh-protocol/FunctionMap.h`

---

## Overview

OpenDriveHub uses **ESP-NOW** as the radio transport.  ESP-NOW operates on a
single WiFi channel (default: channel 1, configurable in
`odh::config::kRadioWifiChannel`) and delivers frames of up to 250 bytes with
no connection setup.

All OpenDriveHub packets share a common 6-byte header followed by a
payload and exactly one checksum byte at the end.

---

## Common Header

```
Offset  Size  Field         Value / Description
──────  ────  ────────────  ──────────────────────────────
0       1     magic[0]      0x4F ('O')
1       1     magic[1]      0x44 ('D')
2       1     version       odh::kProtocolVersion (currently 1)
3       1     type          odh::PacketType enum value
4       2     sequence      Little-endian packet counter
```

The **last byte** of every packet is an XOR checksum of all preceding bytes,
computed by `odh::checksum()`.

---

## Packet Types

```cpp
enum class PacketType : uint8_t {
    Control    = 0x01,  // TX → RX
    Telemetry  = 0x02,  // RX → TX
    Bind       = 0x10,  // TX → RX (connect)
    Ack        = 0x20,  // General acknowledgement
    Announce   = 0x30,  // RX → broadcast (discovery)
    Disconnect = 0x40,  // TX → RX (release link)
};
```

---

## ControlPacket (73 bytes)

Sent by the transmitter at **50 Hz**.  Contains the current function values
and trim offsets for up to 16 functions.

```
Offset  Size  Field                    Description
──────  ────  ───────────────────────  ─────────────────────────────────
0–5     6     header                   Common header (type = 0x01)
6       1     function_count           Number of active FunctionValue entries (≤ 16)
7–70    64    functions[16]            Array of FunctionValue structs (4 bytes each)
71      1     flags                    Reserved
72      1     checksum                 XOR over bytes 0–71
```

### FunctionValue (4 bytes, packed)

```
Offset  Size  Field      Description
──────  ────  ─────────  ──────────────────────────────────────────
0       1     function   odh::Function enum value (0x00–0x0F, 0xFF = None)
1       2     value      RC pulse width in µs [1000, 2000], LE
3       1     trim       Signed trim offset (±100)
```

Each function represents a logical vehicle control (drive, steering, boom,
etc.) rather than a raw channel number.

---

## TelemetryPacket (30 bytes)

Sent by the receiver at **10 Hz** while connected.

```
Offset  Size  Field            Description
──────  ────  ───────────────  ─────────────────────────────────
0–5     6     header           Common header (type = 0x02)
6       2     battery_mv       Battery voltage in mV, LE
8       1     rssi             Receiver RSSI (dBm, signed)
9       1     link_state       odh::LinkState enum value
10      1     model_type       odh::ModelType enum value
11      1     model_flags      Reserved flags
12      1     sensor_count     Number of sensor values (≤ 8)
13–28   16    sensors[8]       16-bit sensor values, LE
29      1     checksum         XOR over bytes 0–28
```

---

## BindPacket (11 bytes)

Sent by the transmitter to initiate a link, or to disconnect.

```
Offset  Size  Field      Description
──────  ────  ─────────  ──────────────────────────────────
0–5     6     header     Common header (type = 0x10 or 0x40)
6       6     mac        Source device MAC address
10      1     checksum   XOR over bytes 0–9
```

---

## AnnouncePacket (28 bytes)

Broadcast by the receiver when in **Disconnected** state.  The transmitter
listens for these packets to discover available vehicles.

```
Offset  Size  Field         Description
──────  ────  ────────────  ──────────────────────────────────
0–5     6     header        Common header (type = 0x30)
6       6     mac           Receiver MAC address
12      1     model_type    odh::ModelType enum value
13      16    name          Vehicle name (null-terminated, max 16 chars)
27      1     checksum      XOR over bytes 0–26
```

---

## Connection Flow

```
Receiver                        Transmitter
────────                        ───────────
   │                                 │
   │ ◄── [power on] ──►             │
   │                                 │
   ├─── AnnouncePacket (broadcast) ──────►│  (every 500 ms)
   ├─── AnnouncePacket ─────────────────►│
   │                                 │
   │   (user taps vehicle on LCD)    │
   │                                 │
   │◄──────── BindPacket ───────────┤  (type = 0x10)
   │                                 │
   │  [state → Connected]           │  [state → Connected]
   │                                 │
   ├─── TelemetryPacket ───────────►│  (10 Hz)
   │◄──────── ControlPacket ────────┤  (50 Hz)
   │                                 │
   │   (user taps Disconnect)       │
   │                                 │
   │◄──────── BindPacket ───────────┤  (type = 0x40)
   │                                 │
   │  [state → Disconnected]        │  [state → Scanning]
   │                                 │
   ├─── AnnouncePacket (broadcast) ──────►│  (resumes)
```

---

## Failsafe

If the receiver does not receive a `ControlPacket` within
`odh::config::kRadioFailsafeTimeoutMs` (default 500 ms), it enters
**Failsafe** state.

In Failsafe:
- All function outputs are set to `odh::config::kFailsafeChannelValue`
  (default 1500 µs = center).
- The receiver continues sending `TelemetryPacket` with
  `link_state = LinkState::Failsafe`.
- If the link is lost for longer than `odh::config::kRadioLinkTimeoutMs`
  (default 3000 ms), the receiver transitions back to **Disconnected** and
  resumes broadcasting `AnnouncePacket`.

---

## Enumerations

### ModelType

```cpp
enum class ModelType : uint8_t {
    Generic   = 0x00,
    DumpTruck = 0x01,
    Excavator = 0x02,
    Tractor   = 0x03,
    Crane     = 0x04,
};
```

### Function

```cpp
enum class Function : uint8_t {
    Drive    = 0x00,  Steering = 0x01,
    Aux1     = 0x02,  Aux2     = 0x03,
    Aux3     = 0x04,  Aux4     = 0x05,
    BoomUD   = 0x06,  BoomLR   = 0x07,
    Bucket   = 0x08,  Swing    = 0x09,
    DumpBed  = 0x0A,  Pto      = 0x0B,
    Hitch    = 0x0C,  Winch    = 0x0D,
    TrackL   = 0x0E,  TrackR   = 0x0F,
    None     = 0xFF,
};
```

### LinkState

```cpp
enum class LinkState : uint8_t {
    Disconnected = 0,
    Binding      = 1,
    Connected    = 2,
    Failsafe     = 3,
    Scanning     = 4,
};
```

---

## Function Map

`firmware/lib/odh-protocol/FunctionMap.h` provides:

- `odh::functionName(Function)` – human-readable label (e.g. `"Boom U/D"`)
- `odh::modelName(ModelType)` – human-readable model label (e.g. `"Excavator"`)
- `odh::defaultFunctionMap(ModelType)` – returns a `FunctionMapping` struct
  with the default set of functions for the given model type
- `odh::functionToChannel()` / `odh::channelToFunction()` – lookup helpers
  between function enums and output channel indices

### Default Mappings

| Model | Functions |
|-------|-----------|
| Generic | Drive(0), Steering(1), Aux1(2), Aux2(3), Aux3(4), Aux4(5) |
| DumpTruck | Drive(0), Steering(1), DumpBed(2) |
| Excavator | TrackL(0), TrackR(1), BoomUD(2), BoomLR(3), Bucket(4), Swing(5) |
| Tractor | Drive(0), Steering(1), Pto(2), Hitch(3) |
| Crane | Drive(0), Steering(1), BoomUD(2), Winch(3), Swing(4) |

---

## Constants

| Constant | Namespace | Value |
|----------|-----------|-------|
| `kMagic0` | `odh::` | `0x4F` ('O') |
| `kMagic1` | `odh::` | `0x44` ('D') |
| `kProtocolVersion` | `odh::` | `1` |
| `kMaxChannels` | `odh::` | `16` |
| `kMaxFunctions` | `odh::` | `16` |
| `kMaxSensors` | `odh::` | `8` |
| `kMaxDiscovered` | `odh::` | `8` |
| `kVehicleNameMax` | `odh::` | `16` |
| `kChannelMin` | `odh::` | `1000` µs |
| `kChannelMid` | `odh::` | `1500` µs |
| `kChannelMax` | `odh::` | `2000` µs |

---

## Legacy Compatibility

`Protocol.h` provides type aliases and constant mappings for older code that
still uses the `OdhControlPacket_t`, `ODH_PKT_*`, `ODH_FUNC_*` naming
convention.  These are defined **outside** the `odh` namespace and will be
removed in a future version.
