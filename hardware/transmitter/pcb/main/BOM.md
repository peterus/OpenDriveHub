# main PCB — Bill of Materials

Brain board of the OpenDriveHub transmitter. See design spec:
`docs/superpowers/specs/2026-08-08-transmitter-main-pcb-design.md`.

**All symbols + footprints are KiCad standard-library parts** — no project-local
symbol/footprint libraries required (verified against KiCad 10.0.5 libs,
2026-08-08). MPNs are Mouser-preferred; reflow-assembly (hotplate + stencil).

## Active parts

| Ref | Qty | Value / Part | KiCad Symbol | KiCad Footprint | MPN (Mouser) | Notes |
|-----|-----|--------------|--------------|-----------------|--------------|-------|
| U1 | 1 | ESP32-S3-WROOM-1U | `RF_Module:ESP32-S3-WROOM-1` | `RF_Module:ESP32-S3-WROOM-1U` | ESP32-S3-WROOM-1U-N16R2 | **Quad-PSRAM** (not R8 Octal — keeps GPIO35-37). External U.FL antenna |
| U2 | 1 | BQ25798 | `Battery_Management:BQ25798` | `Package_DFN_QFN:Texas_RQM0029A_VQFN-29_4x4mm_P0.4mm` | BQ25798RQMR | Buck-boost 2S charger, I²C ADC telemetry. On internal I²C bus (0x6B) |
| U3 | 1 | TPS62933 | `Regulator_Switching:TPS62933` | `Package_TO_SOT_SMD:SOT-583-8` | TPS62933DRLR | 3.8-30V in, 3A buck → 3V3 from SYS |
| U4 | 1 | TCA9548APWR | `Interface_Expansion:TCA9548APWR` | `Package_SO:TSSOP-24_4.4x7.8mm_P0.65mm` | TCA9548APWR | I²C mux 0x70, module bus. A0/A1/A2→GND |
| U5 | 1 | MAX98357A | `Audio:MAX98357A` | `Package_DFN_QFN:TQFN-16-1EP_3x3mm_P0.5mm_EP1.6x1.6mm` | MAX98357AETE+T | I²S class-D amp, 3.3V mono (verify EP size vs datasheet) |
| U6 | 1 | DS3231 | `Timer_RTC:DS3231M` | `Package_SO:SOIC-16W_7.5x10.3mm_P1.27mm` | DS3231SN# | RTC/TCXO on internal I²C bus (0x68). SO-16, pin-verify at capture |
| Q1 | 1 | P-FET reverse-protect | `Device:Q_PMOS_GSD` | `Package_TO_SOT_SMD:SOT-23` | AO3401A | Reverse-polarity block in VBAT path |

## Connectors

| Ref | Qty | Part | KiCad Symbol | KiCad Footprint | Notes |
|-----|-----|------|--------------|-----------------|-------|
| J_USB | 1 | USB-C 16-pin USB2.0 | `Connector:USB_C_Receptacle_USB2.0` | `Connector_USB:USB_C_Receptacle_GCT_USB4085` | VBUS+D±, CC 5.1k sink |
| J_BATT | 1 | XT30 power | `Connector:Conn_01x02_Pin` | (XT30 — verify/local fp) | 2S LiPo main leads |
| J_BAL | 1 | JST-XH 3-pin | `Connector_Generic:Conn_01x03` | `Connector_JST:JST_XH_B3B-XH-A_1x03_P2.50mm_Vertical` | 2S balance |
| J_MOD0..7 | 8 | JST-XH 5-pin | `Connector_Generic:Conn_01x05` | `Connector_JST:JST_XH_B5B-XH-A_1x05_P2.50mm_Vertical` | Module sockets (3V3,GND,SDA,SCL,/INT) |
| J_JOYL, J_JOYR | 2 | JST-PH 5-pin | `Connector_Generic:Conn_01x05` | `Connector_JST:JST_PH_B5B-PH-K_1x05_P2.00mm_Vertical` | Joysticks (3V3,GND,X,Y,SW) |
| J_SPK | 1 | JST-PH 2-pin | `Connector_Generic:Conn_01x02` | `Connector_JST:JST_PH_B2B-PH-K_1x02_P2.00mm_Vertical` | Speaker |
| J_DISP | 1 | 2.54 mm 1×14 header | `Connector_Generic:Conn_01x14` | `Connector_PinHeader_2.54mm:PinHeader_1x14_P2.54mm_Vertical` | Display+touch harness |
| J_SD | 1 | microSD push-push | `Connector:microSD_Card_Det` | `Connector_Card:microSD_HC_Hirose_DM3AT-SF-PEJM5` | Shared SPI2 + SD_CS |
| J_DBG | 1 | 2.54 mm 1×4 header | `Connector_Generic:Conn_01x04` | `Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical` | 3V3, TX(43), RX(44), GND |
| J_ANT | 1 | U.FL/IPEX | `Connector_Coaxial:U.FL` | `Connector_Coaxial:U.FL_Hirose_U.FL-R-SMT-1_Vertical` | External 2.4 GHz antenna |
| SW_BOOT, SW_RST, SW_PWR | 3 | tact 6 mm | `Switch:SW_Push` | `Button_Switch_THT:SW_PUSH_6mm_H5mm` | Boot(GPIO0)/Reset(EN)/Ship-mode(/QON) |

## Passives (values final at schematic capture; footprints 0805 for hand/reflow)

| Ref group | Qty | Value | Symbol | Footprint | Purpose |
|-----------|-----|-------|--------|-----------|---------|
| R (I²C0 pull-ups) | 2 | 4.7k | `Device:R` | `Resistor_SMD:R_0805_2012Metric` | Module-bus trunk SDA/SCL |
| R (I²C1 pull-ups) | 2 | 4.7k | `Device:R` | 0805 | Internal-bus SDA/SCL |
| R (mux ch pull-ups) | 16 | 4.7k | `Device:R` | 0805 | 8× channel SDA/SCL pairs |
| R (module INT) | 8 | 10k | `Device:R` | 0805 | MOD_INT0..7 pull-ups |
| R (touch INT) | 1 | 10k | `Device:R` | 0805 | TOUCH_INT pull-up |
| R (MUX_RST) | 1 | 10k | `Device:R` | 0805 | TCA9548A /RESET default-high |
| R (CC) | 2 | 5.1k | `Device:R` | 0805 | USB-C CC1/CC2 sink |
| R (EN) | 1 | 10k | `Device:R` | 0805 | ESP32 EN pull-up |
| R (DISP_BL pd) | 1 | 10k | `Device:R` | 0805 | GPIO46 strap pull-down |
| R (charger sense) | 2 | per BQ25798 | `Device:R` | per datasheet | RAC / RBAT sense |
| C (EN) | 1 | 1µF | `Device:C` | 0805 | ESP32 EN RC |
| C (decoupling) | ~20 | 100nF | `Device:C` | 0805 | per IC VDD pin |
| C (bulk) | ~6 | 10µF | `Device:C` | 0805/1206 | module + rails |
| C (charger/buck) | set | per datasheet | `Device:C` | 0805/1206 | VBUS/SYS/BAT/boot/REGN caps |
| L1 | 1 | per BQ25798 | `Device:L` | per datasheet | Charger power inductor |
| L2 | 1 | per TPS62933 | `Device:L` | per datasheet | 3V3 buck inductor |
| BT1 | 1 | CR1220 + holder | `Device:Battery_Cell` | `Battery:BatteryHolder_Keystone_3001_1x12mm` | DS3231 backup |

## I²C address map

- **Module bus (I²C0):** TCA9548A `0x70`; behind mux Ch0-7: PCF8574A `0x38` each (isolated per channel).
- **Internal bus (I²C1):** FT6236 touch `0x38` (on display module), BQ25798 `0x6B`, DS3231 `0x68`. No collision.

## Net summary

See design spec §2 (block diagram), §4 (I²C topology), §6 (frozen GPIO pin map),
§8 (connector pinouts). Module socket pinout: 1=3V3, 2=GND, 3=SDA, 4=SCL, 5=/INT.

## Verify-at-capture (spec §12)

- MAX98357A TQFN EP size vs. datasheet; SD_MODE gain/mono resistor value.
- DS3231 SO-16 pin mapping (symbol DS3231M vs. DS3231SN part).
- BQ25798 external network (inductor, sense R, ACFET/BATFET) per TI SLUSD90.
- TPS62933 feedback divider for 3.3 V.
- XT30 footprint (may need project-local `.pretty` if not in std libs).
- microSD footprint variant vs. actual socket in hand.
