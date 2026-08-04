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

## Follow-up fix (v3.4.3): Core2 touch/button freeze

`maintain_connection()` running every `loop()` iteration meant the blocking
3s `wait_for_connection()` verify-loop (added above) could now fire from the
background path too. On Core2, `connect_wifi()` also does extra
`esp_wifi_stop()/start()` housekeeping with hard `delay()`s, so a failed
reconnect attempt (e.g. real device ID configured but no/wrong WiFi
credentials) froze `loop()` for ~3.3-3.8s every 10s — long enough that
`M5.update()` (which polls touch) never got called between touches, making
menu buttons on Core2 appear unresponsive. CoreS3 has no equivalent
Core2-only delays and wasn't affected.

Fix: `WiFiWrapper::connect_wifi()` gained a `wait_for_result` parameter.
`maintain_connection()` (the background/automatic path) now passes `false`
— it kicks off `WiFi.begin()`/`WiFi.reconnect()` and returns immediately
without blocking `loop()`, relying on the next periodic call to observe
whether the connection succeeded. The WiFi settings screen (user-initiated,
where blocking briefly for feedback is expected) keeps `wait_for_result`
defaulted to `true`.

## Follow-up fix (v3.4.4): blank screen entering Real-Time mode on Core2

`GFXScreen`'s screen-switch sequence (`gfx_screen.cpp`) clears the display
*before* calling the new screen's `enter_screen()`. `FixedModeScreen::enter_screen()`
(`screens/fixed_mode.cpp`) called `WiFiWrapper_i.connect_wifi(...)` with the
default `wait_for_result=true`, so on Core2 the same 3s verify-loop (plus
Core2's `esp_wifi_stop()/start()` delays) ran synchronously right after the
screen was blanked — a ~3s blank screen every time Real-Time mode was
entered while WiFi wasn't already connected.

Fix: `FixedModeScreen::enter_screen()` now passes `wait_for_result=false`.
The Core2-specific WiFi init delays (mode/start housekeeping, needed to
avoid crashes on an uninitialized radio) still run, but the multi-second
connect-verify wait is skipped; `maintain_connection()` picks up and
confirms the connection on a later background pass.
