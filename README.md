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

- M5Stack Core device
- LND-7317 radiation sensor
- GPS module
- SD card for logging

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

## SAFEZEN.txt configuration (SD card)

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
- `display_cpm`
  - Display unit selector. `1` = show CPM. `0` = show µSv/h.
- `manual_logging`
  - Logging behavior. `1` = manual start/stop. `0` = automatic depending on mode/movement.
- `enable_journal`
  - Enable the journal log. `1` = enabled, `0` = disabled.
- `log_void`
  - Include invalid/void lines in logs. `1` = include, `0` = exclude.
- `screen_dim_timeout`
  - Seconds of inactivity before screen dims.
- `screen_off_timeout`
  - Seconds of inactivity before screen turns off.
- `animated_screensaver`
  - Screensaver animation on dim/off. `1` = enabled, `0` = disabled.
- `error_alert_sound`
  - Error beep sounds. `1` = enabled, `0` = disabled.
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

Notes:
- Unknown or unsupported lines are ignored by the firmware.
- Settings changed from the device UI are persisted to internal memory and can be written back to `SAFEZEN.txt` via Settings → “Save to SD”.
- The device also supports configuring via Wi‑Fi using the built-in Access Point or a local network (see Settings screen).

## Contributing

We welcome contributions! Please follow these steps:

1. Fork the repository.
2. Create a feature branch.
3. Submit a Pull Request.

## License

This project is licensed under [appropriate license].

## Support

- Visit [Safecast.org](https://safecast.org)
- Join our community forums
- Report issues on GitHub

## Credits

Developed and maintained by the Safecast community.

This repo is for development of the bGeigieZen.
Specs can be found at https://github.com/Safecast/bGeigieZen/wiki/Specification
Much more information and documentation for the bGeigieZen is at https://bgeigiezen.safecast.jp
