# WiFi reconnect fix

## Problem

The WiFi icon correctly shows red when disconnected, but the device would
never reconnect on its own. Root causes:

1. The only code path that could call `WiFi.reconnect()`/`WiFi.begin()` was
   `ApiConnector::handle_produced_work()`, which is gated behind fresh/valid
   GPS+GM sensor data and a send-frequency timer. If sensor data wasn't
   flowing, reconnection was never attempted.
2. No `WiFi.onEvent()` handler existed, so a disconnect was only ever
   noticed the next time that narrow polling path happened to run.
3. `connect_wifi()` fired `WiFi.reconnect()`/`WiFi.begin()` then did a single
   `delay(100)` and gave up — not enough time for the radio to actually
   reassociate before the attempt was judged failed.

## Fixes

- **`utils/wifi_connection.{h,cpp}`**: added `WiFiWrapper::register_events()`
  which registers a `WiFi.onEvent()` callback for
  `ARDUINO_EVENT_WIFI_STA_DISCONNECTED`, flagging the drop immediately
  (`flag_disconnect_event()` / `consume_disconnect_event()`). Replaced the
  single `delay(100)` after `WiFi.reconnect()`/`WiFi.begin()` with
  `wait_for_connection(3000)`, a bounded poll loop that returns as soon as
  `WL_CONNECTED` is reached (or times out after 3s).

- **`handlers/api_connector.{h,cpp}`**: added `ApiConnector::maintain_connection()`,
  called every `loop()` iteration regardless of sensor data state. It skips
  only when no device ID is configured or the AP config screen is active,
  consumes any pending disconnect event to skip the backoff, and otherwise
  calls the existing `activate(true)` (10s backoff) to retry.

- **`main.cpp`**: registers the WiFi event handler in `setup()` and calls
  `api_connector.maintain_connection()` at the top of `loop()`, so
  reconnection is no longer starved by GPS/GM data gating.

## Net effect

- A dropped link is detected immediately via the event handler.
- Reconnection is retried periodically in the background, independent of
  sensor data freshness.
- Each reconnect attempt gets a real bounded window to succeed instead of
  a single 100ms check.
