# SAFEZEN.txt

<<<<<<< HEAD
Every Zen device requires an SD card to work. On that SD card there must be a file named `SAFEZEN.txt` at the SD root with your device settings.

See also the full documentation in the root `README.md` under “SAFEZEN.txt configuration (SD card)”.

## Samples

- Minimal configuration: `SDcard/minimal/SAFEZEN.txt` (only `device_id`)
- All settings example: `SDcard/all_settings/SAFEZEN.txt` (includes legacy keys; see notes below)

## Minimum required

- `device_id`
  - Required for the device to operate. Must be between 5000 and 5999.

## Supported settings (key=value)

- `version` — Optional firmware version marker written by the device.
- `device_id` — Required device identifier (5000–5999).
- `user_name` — Optional display/owner name.
- `api_key` — Optional Safecast API key (if uploading is used).
- `access_point_password` — Password for the device’s Wi‑Fi Access Point.
- `wifi_ssid`, `wifi_password` — Primary Wi‑Fi credentials.
- `wifi_ssid2`, `wifi_password2` — Secondary Wi‑Fi credentials (optional).
- `wifi_profile` — Active Wi‑Fi profile selector: `1` = primary, `2` = secondary.
- `alert_threshold` — CPM alert threshold (10–9999). Triggers audible + visual “CPM ALERT”.
- `display_cpm` — Display unit: `1` = CPM, `0` = µSv/h.
- `manual_logging` — Logging behavior: `1` = manual, `0` = automatic.
- `enable_journal` — Journal log: `1` = enabled, `0` = disabled.
- `log_void` — Include invalid/void lines in logs: `1` = include, `0` = exclude.
- `screen_dim_timeout` — Seconds of inactivity before screen dims.
- `screen_off_timeout` — Seconds of inactivity before screen turns off.
- `animated_screensaver` — Screensaver animation: `1` = enabled, `0` = disabled.
- `error_alert_sound` — Error beep sounds: `1` = enabled, `0` = disabled.
- `dim_brightness` — Brightness (0–100) when dimmed/screensaver is active.
- `audio_volume` — Global audio volume (0–100) for clicks and alerts.
- `fixed_latitude`, `fixed_longitude` — Coordinates used in Fixed mode.
- `fixed_range` — Radius (km) for Fixed mode validity.
- `dop_max` — Maximum acceptable GPS DOP value for fix validity.

Notes
- Settings changed via the device UI are saved to internal memory and can be written back to `SAFEZEN.txt` from Settings → “Save to SD”.
- Unknown or unsupported keys are ignored by the firmware parser.

## Legacy/compatibility keys

Some older sample files (including `all_settings/SAFEZEN.txt`) contain keys that are not used by the current firmware but are safely ignored when reading:

- `ush_divider`, `cpmn`, `bqm_factor`, `bqmn`, `country_code`, `gt`, `gm`, `sensor_type`, `sensor_shield`, `sensor_mode`
- `alarm_threshold` — Legacy name for `alert_threshold`. The current firmware writes/uses `alert_threshold`.

It is safe to leave these lines present; they will be skipped by the parser. New files written by the device will use the current keys.
=======
Every Zen device requires an SD card to work. On that SD card there should be a 
file named SAFEZEN.txt with personal settings.

There are 3 example files, the minimal required settings is device_id 
(see [SAFEZEN_minimal.txt](SAFEZEN_minimal.txt)). Device id must be greater than 0 to work

Other settings can be added, see [SAFEZEN_all_settings.txt](SAFEZEN_all_settings.txt) 
for full options.

All settings can be changed (or will be added later) in the config screen of the device. 

More info will be added in this readme at a later time.
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
