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

## Follow-up fix: touch/button freeze from background reconnect

`maintain_connection()` running every `loop()` iteration meant the blocking
3s `wait_for_connection()` verify-loop (added above) could now fire from the
background path too. A failed reconnect attempt (e.g. device ID configured
but no/wrong WiFi credentials) froze `loop()` for up to 3s every 10s (worse
on Core2, which also runs extra `esp_wifi_stop()/start()` housekeeping with
hard `delay()`s) — long enough that `M5.update()` (which polls touch/buttons)
never got called between touches. On Core2 this made menu buttons appear
unresponsive; on CoreS3 it could make the screen appear un-wakeable from
screensaver/blanked state, since the wake tap is never sampled.

Fix: `WiFiWrapper::connect_wifi()` gained a `wait_for_result` parameter.
`maintain_connection()` (the background/automatic path) now passes `false`
— it kicks off `WiFi.begin()`/`WiFi.reconnect()` and returns immediately
without blocking `loop()`, relying on the next periodic call to observe
whether the connection succeeded. The WiFi settings screen (user-initiated,
where blocking briefly for feedback is expected) keeps `wait_for_result`
defaulted to `true`.

**Superseded by the rewrite below** — the `wait_for_result` flag design was
abandoned in v3.4.3 because every call site had to remember to opt out of
blocking, and two were missed (see below).

## Rewrite (v3.4.3): remove the blocking wait entirely

Across three follow-up patches (touch freeze, then RT-mode blank screen,
twice), the root problem kept resurfacing: `connect_wifi()` could still
block for up to 3s via `wait_for_connection()`, a `while` loop polling
`WiFi.status()` every 100ms — not a literal `delay(3000)`, but functionally
the same for the caller, since it never returns control to `loop()` until
connected or timed out. Every new call site (`ApiConnector::activate()`,
`FixedModeScreen::enter_screen()`, `LogViewerScreen::enter_screen()`, the
WiFi settings screen) had to explicitly opt out via `wait_for_result=false`
to avoid it, and it was easy to miss one — which is exactly what kept
happening.

The fix: `connect_wifi()` no longer has a blocking mode at all.
`wait_for_connection()` and the `wait_for_result` parameter are gone.
`connect_wifi()` fires `WiFi.begin()`/`WiFi.reconnect()` and returns
immediately, always — no call site can opt back into blocking, so no call
site can regress.

The state that the blocking wait used to track (is an attempt in flight,
has it timed out) moved into `WiFiWrapper` using an `RBD::Timer`
(`alextaujenis/RBD_Timer`, already a dependency — used the same way for
GPS fix-age tracking in `gps_connector.h`):
- `connect_wifi()` calls `_connect_timer.restart()` when it fires an attempt.
- `WiFiWrapper::connecting()` — true while an attempt is in flight and the
  8s timeout (`WIFI_CONNECT_TIMEOUT_MS`) hasn't elapsed.
- `WiFiWrapper::connect_timed_out()` — true once that timeout elapses
  without connecting.

Callers that used to get synchronous feedback now poll instead. The WiFi
settings screen's local-network page (`screens/wifi_settings.cpp`) now
calls `force_next_render()` from `handle_input()` every tick while
`!wifi_connected()`, so the "Connected: ..." status line updates live as
the async attempt resolves (showing "Connecting..." then "Yes" or
"Timed out"), rather than only rendering once on screen entry.

The pre-existing Core2 `esp_wifi_stop()/start()` housekeeping and its
`delay(50/100/200)` calls (~300-700ms total, guarding against crashes on an
uninitialized radio) were left alone — those are a separate, much smaller
cost and not what was causing the multi-second freezes.
