# bGeigieZen

A modern radiation monitoring device based on the M5Stack hardware platform, developed by the Safecast community.

## Overview

bGeigieZen is a portable radiation monitoring device that combines precision sensing with a user-friendly interface. Built on the M5Stack platform, it provides real-time radiation measurements with GPS logging capabilities.
![from website](https://github.com/user-attachments/assets/a8260427-227d-4a53-8914-8a7f9e7631ee)

## Features

- Real-time radiation monitoring
- GPS location tracking
- Data logging to SD card
- User-friendly display interface
- Long battery life
- Mobile app connectivity
- Compatible with the Safecast API

## Hardware Requirements

- M5Stack Core device (Core2 or CoreS3)
- LND-7317 radiation sensor
- GPS module (see [Supported GPS Modules](#supported-gps-modules) below)
- SD card for logging

## Supported GPS Modules

The firmware supports u-blox GPS modules over UART (Port C on M5Stack).

| Module | Chip | Protocol | Mode | Notes |
|---|---|---|---|---|
| Beitian BN-280 / BN-880 | u-blox M8 | UBX (bidirectional) | Full UBX | Default recommended module |
| u-blox NEO-M9N | u-blox M9 | UBX (bidirectional) | Full UBX | |
| u-blox NEO-M10 | u-blox M10 | UBX (bidirectional) | Full UBX | |
| u-blox NEO-7M / UBX-G7020-KT | u-blox M7 | NMEA (RX-only) or UBX | NMEA fallback or Full UBX | RX-only works with only GPS TX → CoreS3 GPIO18 connected; connect GPS RX → GPIO17 for full UBX |

### Wiring (M5Stack CoreS3 Port C)

| CoreS3 Port C pin | GPIO | Direction | Connect to |
|---|---|---|---|
| RXD | GPIO18 | ESP32 receives | GPS TX |
| TXD | GPIO17 | ESP32 transmits | GPS RX (optional — required for full UBX mode) |

**NMEA fallback mode:** If only the GPS TX → GPIO18 wire is connected (GPS RX is not wired), the firmware automatically detects this and switches to NMEA-only parsing. Coordinates, time, date, and satellite count are still reported. The status bar shows `GPS?` while searching, then the satellite count and fix state once acquired. Full UBX mode (both wires) is recommended for faster cold-start and richer data.

## Software Setup

1. **Install Required Development environment**
   - VS Code with PlatformIO or PlatformIO standalone

2. **Install Required Libraries**:
   - m5stack/M5Unified
   - claypuppet/SensorReporter
   - beakes/TeenyUbloxConnect
   - alextaujenis/RBD_Timer

3. **Clone the Repository**:
   ```bash
   git clone https://github.com/Safecast/bGeigieZen.git
   ```

## Building and Flashing

1. Open the project in VS Code with PlatformIO, or invoke PlatformIO from the shell/CMD.
2. Select your M5Stack board.
3. Compile and upload.

## Usage

1. Power on the device.
2. Wait for GPS signal acquisition.
3. Radiation measurements will display on screen.
4. Data logs automatically save to the SD card.

## Status Bar

The bottom status bar shows live device state at a glance:

| Indicator | Meaning |
|---|---|
| `GPS?` (orange) | GPS active, searching for satellites (cold start) |
| `GPS12` (orange) | GPS tracking 12 satellites, no fix yet |
| `GPS5` (green) | GPS fix acquired using 5 satellites |
| `GPS` (red) | GPS module not detected / error |
| `GM` (white) | Geiger tube active |
| `GM` (red) | Geiger tube required but not detected |
| `SD` (white/green) | SD card present and writeable |
| `SD` (red) | SD card required but missing or unreadable |
| `WF` (white/green) | Wi-Fi connected |
| `BT` (white/green) | Bluetooth active |
| `SN` (green) | Sound enabled |
| `#12345` | Device ID |

## SD card configuration files

This project references configuration files used by both bGeigie Nano and bGeigie Zen devices. They are different and not interchangeable. Use the appropriate file for your device:

### bGeigie Nano — `SAFECAST.TXT`

- Location: SD card root of a bGeigie Nano.
- Format: `key=value` per line. Example reference file: https://github.com/Safecast/bGeigieNanoKit/blob/master/SD%20card/SAFECAST.TXT

Common keys (with firmware semantics):
- `nm` — Profile name. Typically `SAFECAST`. Informational label, shown in menus/log headers.
- `did` — Device ID used by the Safecast ecosystem to associate logs/uploads. Required for proper identification. Range and format depend on Nano firmware and Safecast policy.
- `gt` — GPS type selector. `0` often means u‑blox; other values map to alternative GPS chipsets/modules. Used to configure serial baud, message set, and initialization.
- `gm` — GPS mode. Controls GPS operating/profile mode (e.g., cold/warm start preferences or power mode). Exact mapping is firmware‑specific.
- `cpmf` — CPM calibration factor (numeric). Multiplier applied to raw tube counts to compute calibrated CPM or uSv/h conversion path.
- `cpmn` — CPM factor name (text). Label for the calibration factor (e.g., `Cs137`) stored alongside logs.
- `bqmf` — Bq/m² calibration factor (numeric). Multiplier used when exporting in Bq/m² units.
- `bqmn` — Bq/m² factor name (text). Label for the Bq/m² factor (e.g., `Cs137`).
- `al` — Alert threshold in CPM. When current CPM ≥ `al`, the Nano emits an alert (beep/visual), depending on sound settings.
- `tz` — Timezone offset in hours from UTC. Example: `9` for JST. Used for local time stamping on device.
- `cn` — Country code (ISO‑3166). Often `JPN`, etc. May be embedded in file headers or used for region‑specific defaults.
- `st` — Logging start mode. `0` typically manual; non‑zero values may enable auto‑start or different trigger logic. Firmware‑dependent.
- `ss` — Sound setting. `0` off, `1` on (beeps/clicks). May offer additional levels on some firmware.
- `sh` — Screen brightness (0–100). Backlight level.
- `sm` — Screen/map mode. Selects UI layout or mapping behavior. Firmware‑dependent.
- `cw` — Commit/write interval in seconds. Controls how frequently log lines are flushed to SD.

Notes:
- Values and enumerations above reflect common Nano firmwares; refer to the Nano documentation for the definitive mapping.
- Consult the Nano repository for full semantics and latest defaults.

Sample templates:

- Minimal `SAFECAST.TXT` (Nano):

```ini
nm=SAFECAST
did=5555
al=100
tz=9
ss=1
cw=60
```

- Full `SAFECAST.TXT` example (Nano):

```ini
nm=SAFECAST
did=5555
gt=0
gm=0
cpmf=334
cpmn=Cs137
bqmf=37
bqmn=Cs137
al=100
tz=9
cn=JPN
st=0
ss=1
sh=100
sm=0
cw=60
```

References:
- Canonical example file in Nano kit: https://github.com/Safecast/bGeigieNanoKit/blob/master/SD%20card/SAFECAST.TXT
- Nano project docs and firmware should be consulted for exact enumerations and behavior.

### bGeigie Zen — `SAFEZEN.txt`

Every Zen device uses an SD card with a `SAFEZEN.txt` configuration file placed at the SD root. At minimum, you must set a valid `device_id` for the device to operate.

Quick start:
- See sample files in `SDcard/`:
  - `SDcard/minimal/SAFEZEN.txt` (minimal config with just `device_id`)
  - `SDcard/all_settings/SAFEZEN.txt` (example of all commonly supported fields)
- You can also manage all settings from the device: Menu → Settings (the device can read and write `SAFEZEN.txt`).

Supported settings (key=value per line):
- `version`
  - Optional firmware version marker written by the device. If missing, the current firmware attempts to parse using the latest format.
- `device_id`
  - Required. Must be between 5000 and 5999. The device will not operate without a valid ID in this range.
- `user_name`
  - Optional display and identification name used on-screen and in some outputs.
- `api_key`
  - Optional Safecast API key for uploading where applicable.
- `access_point_password`
  - Password for the device’s built-in Wi‑Fi Access Point (used by the configuration portal).
- `wifi_ssid`
  - Primary Wi‑Fi SSID for connecting to a local network.
- `wifi_password`
  - Password for `wifi_ssid`.
- `wifi_ssid2`
  - Secondary Wi‑Fi SSID profile (optional).
- `wifi_password2`
  - Password for `wifi_ssid2`.
- `wifi_profile`
  - Select active Wi‑Fi profile. `1` = use `wifi_ssid`/`wifi_password`. `2` = use `wifi_ssid2`/`wifi_password2`.
- `alert_threshold`
  - CPM alert threshold. Range 10–9999. Triggers audible and visual “CPM ALERT” when exceeded.
  - Note: Older examples may show `alarm_threshold`, but the current firmware uses `alert_threshold`.
  - See: [Zen Specification — Alerts](https://github.com/Safecast/bGeigieZen/wiki/Specification)
- `display_cpm`
  - Display unit selector. `1` = show CPM. `0` = show µSv/h.
- `manual_logging`
  - Logging behavior. `1` = manual start/stop. `0` = automatic depending on mode/movement.
  - See: [Zen Specification — Logging](https://github.com/Safecast/bGeigieZen/wiki/Specification)
- `enable_journal`
  - Enable the journal log. `1` = enabled, `0` = disabled.
  - See: [Zen Specification — Files & Journaling](https://github.com/Safecast/bGeigieZen/wiki/Specification)
- `log_void`
  - Include invalid/void lines in logs. `1` = include, `0` = exclude.
- `screen_dim_timeout`
  - Seconds of inactivity before screen dims.
  - See: [Zen Specification — Display & Power](https://github.com/Safecast/bGeigieZen/wiki/Specification)
- `screen_off_timeout`
  - Seconds of inactivity before screen turns off.
  - See: [Zen Specification — Display & Power](https://github.com/Safecast/bGeigieZen/wiki/Specification)
- `animated_screensaver`
  - Screensaver animation on dim/off. `1` = enabled, `0` = disabled.
  - See: [Zen Specification — Display & Power](https://github.com/Safecast/bGeigieZen/wiki/Specification)
- `error_alert_sound`
  - Error beep sounds. `1` = enabled, `0` = disabled.
  - See: [Zen Specification — Alerts](https://github.com/Safecast/bGeigieZen/wiki/Specification)
- `dim_brightness`
  - Brightness level (0–100) used while dimmed/screensaver.
- `audio_volume`
  - Global audio volume (0–100) for clicks and alerts.
- `fixed_latitude`
  - Latitude used in Fixed mode.
- `fixed_longitude`
  - Longitude used in Fixed mode.
- `fixed_range`
  - Radius in km for Fixed mode validity.
- `dop_max`
  - Maximum acceptable Dilution of Precision (DOP) for GPS fix validity.
  - See: [Zen Specification — GPS & Accuracy](https://github.com/Safecast/bGeigieZen/wiki/Specification)

Notes:
- Unknown or unsupported lines are ignored by the firmware.
- Settings changed from the device UI are persisted to internal memory and can be written back to `SAFEZEN.txt` via Settings → “Save to SD”.
- The device also supports configuring via Wi‑Fi using the built-in Access Point or a local network (see Settings screen).

### Nano ↔ Zen quick comparison (overlapping concepts)

| Concept | bGeigie Nano (SAFECAST.TXT) | bGeigie Zen (SAFEZEN.txt) |
| --- | --- | --- |
| Device ID | `did` (format/range Nano‑specific) | `device_id` (required, 5000–5999) |
| Alert threshold (CPM) | `al` | `alert_threshold` |
| Timezone | `tz` (hours offset) | not required; Zen uses RTC and can show local time; timezone not needed for logging |
| Display units | n/a (Nano UI dependent) | `display_cpm` (1=CPM, 0=µSv/h) |
| Wi‑Fi/API | n/a in Nano SAFECAST | `wifi_*`, `api_key`, `access_point_password` |
| Logging behavior | `st`/`cw` (start mode and commit interval) | `manual_logging` (manual vs auto), commit policy handled by firmware |
| GPS accuracy threshold | n/a (implicit) | `dop_max` (DOP limit) |
| Fixed location | n/a | `fixed_latitude`, `fixed_longitude`, `fixed_range` |
| Brightness & power | `sh` (0–100) | `dim_brightness`, `screen_dim_timeout`, `screen_off_timeout`, `animated_screensaver` |
| Sound & alerts | `ss` (0/1) | `error_alert_sound`, `audio_volume`, `alert_threshold` |

Tip: These files are not interchangeable. Use `SAFECAST.TXT` on Nano, and `SAFEZEN.txt` on Zen.

## Contributing

We welcome contributions! Please follow these steps:

1. Fork the repository.
2. Create a feature branch.
3. Submit a Pull Request.

## License

This project is licensed under the MIT License.

## Support

- Visit [Safecast.org](https://safecast.org)
- Join our community forums
- Report issues on GitHub

## Credits

Developed and maintained by the Safecast community.

This repo is for development of the bGeigieZen.
Specs can be found at https://github.com/Safecast/bGeigieZen/wiki/Specification
Much more information and documentation for the bGeigieZen is at https://bgeigiezen.safecast.jp

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/Safecast/bGeigieZen)
