# Real-time trip logging and upload (`rt-drive`)

Available from firmware 3.4.4 beta.

## What it does

In **Real-time** mode the Zen posts readings live to the API while it sits at home. When it leaves the home fence
("Roaming") it also records a log file on the SD card. Back home, with WiFi, the recorded trip is uploaded to
**api.safecast.org** and **simplemap.safecast.org**.

## Trip states

Handled by `RtTripHandler` ([handlers/rt_trip_handler.cpp](../bgeigiezen_firmware/handlers/rt_trip_handler.cpp)).
It runs only when the log aggregator produces fresh data (about every 5 s); nothing runs in the main loop.
It is switched on in `FixedModeScreen::enter_screen` and off in `leave_screen`.

| State | Meaning | Next |
|---|---|---|
| `e_trip_home` | At home, nothing pending | Roaming for 10 s with a valid GPS fix: start the `rt_drive` logger, go to recording |
| `e_trip_recording` | Left the fence, logging to `/rt-drive/` | At home for 10 s (valid fix), or SD logger stopped: go to upload |
| `e_trip_upload` | Trips waiting to be uploaded | When nothing is left: go to home. If roaming again for 10 s: record the next trip first |

- Roaming = valid GPS fix (no fallback position) outside the home fence. No fix keeps the current state, so indoors
  without GPS a trip never starts and a running trip never ends.
- 10 s debounce (`RT_TRIP_DEBOUNCE_MS`) avoids flapping at the fence edge.
- A trip file with fewer than 2 lines (`MIN_LOG_LINES_FOR_KEEP`) is deleted.
- Leaving Real-time mode, or removing the SD card, closes the trip log.
- Files left on the SD card (power loss) are uploaded the next time RT mode starts.

## SD card folders

| Folder | Contents |
|---|---|
| `/rt-drive/` | Trip being recorded, and trips not yet uploaded |
| `/rt-drive/api-done/` | Sent to api.safecast.org, not yet to simplemap |
| `/rt-drive/map-done/` | Sent to simplemap (because the API failed), not yet to api.safecast.org |
| `/rt-drive/uploaded/` | Sent to both. Permanent record, never deleted automatically |
| `/rt-drive/uploads.log` | Text log, one line per upload result (see below) |

File names are `YYYY-MM-DD_HHMM.log` (trip start time from the RTC), or `latest.log` if the RTC is not valid. The log
header says `# Mode: Real-time trip` and `format=... beta-zen/rt-drive`. Which folder a file is in is its upload status.
`/rt-drive` is not cleared by the SD wipe function.

## Upload

One file per data event, while WiFi is connected. Uploads run in a background task (the screen never blocks). api.safecast.org and simplemap are independent: when one fails, the other is still tried, and each retries on its own after 60 s (`RT_UPLOAD_RETRY_MS`). A file that keeps failing does not block other files. An API reply "md5sum is the same as of Bgeigie Import N" (422) counts as uploaded.
A file only moves on to the next folder after an HTTP 2xx reply.

1. **api.safecast.org**: `POST https://api.safecast.org/bgeigie_imports.json?api_key=<key>`, multipart field
   `bgeigie_import[source]`. Only the API key is needed.
2. **simplemap.safecast.org**: login with `POST /api/auth/login` `{"email","api_key"}` to get a session cookie,
   then `POST /upload` with multipart field `files[]` and that cookie. Needs the account email **and** the same API key.

API goes first, so simplemap sees the file already exists on api.safecast.org and does not forward a duplicate.
Code: [utils/log_upload.cpp](../bgeigiezen_firmware/utils/log_upload.cpp). The log viewer's Upload button uses the same
function for api.safecast.org. TLS certificates are not pinned yet (`setInsecure()`, same as before).

## Settings

| Setting | Where | Notes |
|---|---|---|
| API key | Config page, SD `api_key=` | Same key for api.safecast.org and simplemap |
| Simplemap email | Config page ("Simplemap email"), SD `simplemap_email=` | Email of the simplemap account. Without it the simplemap step fails (`http=0`) and the file stays in `api-done/` |
| Home latitude/longitude, fixed range | Config page, SD | Home 0 or range 0 = "no home": always roaming |

The info screen (Connection settings) shows the API key and the simplemap email.

### Simplemap email on the SD card

`SAFEZEN.txt` at the SD root takes one `key=value` per line:

```
device_id=5002
api_key=<your Safecast API key>
simplemap_email=you@example.com
```

- The email is the one your **simplemap.safecast.org** account is registered with (not necessarily your Safecast or
  Gmail address; a wrong email gives "Invalid email or API key" on login).
- The line must come after `device_id` (settings are only read once the device id is known). No spaces in the value,
  shorter than the device's maximum setting length.
- Read at boot. Values entered on the config web page are stored in the device memory and written back to
  `SAFEZEN.txt` by Settings -> "Save to SD" (the key `simplemap_email` is included in that file).
- If the line is missing or empty the email stays empty: api.safecast.org uploads still work, simplemap uploads
  fail with `http=0` in `uploads.log` and the file waits in `/rt-drive/api-done/`.
- Factory reset clears the email together with the other settings.
- Samples: `SDcard/SAFEZEN.txt`, `SDcard/SAFEZEN_333.txt`, `SDcard/all_settings/SAFEZEN.txt`.

## Upload log on the SD card (`/rt-drive/uploads.log`)

Every upload result is appended as one line to `/rt-drive/uploads.log` (created automatically):

```
2026-11-04T14:11:02Z api 2026-11-04_1403.log http=201 id=74020
2026-11-04T14:11:09Z simplemap 2026-11-04_1403.log http=200 id=a0nJzy
2026-11-04T14:20:41Z simplemap 2026-11-04_1410.log http=401
```

- Fields: time (from the last GPS/RTC reading), service (`api` or `simplemap`), file name, HTTP result, and the id
  returned on success: the import id on api.safecast.org, the track id on simplemap (note: simplemap may reuse an
  existing track id when the readings are already known).
- Success = `http=200..299`. Anything else is a failure: `401` = login or key rejected, `0` = request could not be
  made (missing email/key, no file), `429`/`5xx` = server side.
- A failure is written once per file and result, not on every 60 s retry. A different failure code on a later retry
  is written again. The final success line is always written.
- The file is written on every trip upload, independent of the journal setting. It is a plain text log with no
  size limit; delete it freely, it is recreated on the next upload.
- The folder a file is in (`/rt-drive/`, `api-done/`, `uploaded/`) remains the upload status; `uploads.log` is the
  history.
- Never uploaded itself: the trip scan only picks up `*.log` trip files and skips `uploads.log`. Not used as a queue: the Zen never reads `uploads.log`, so deleting it cannot cause files to be re-uploaded.

## Duplicates

- Simplemap keeps real-time data in its own table, separate from track markers. A trip log does not conflict with it.
- Simplemap compares up to 128 readings of an upload with existing tracks. With 10 or more matches the import is
  skipped and the existing track is reused; identical rows are never stored twice.
- api.safecast.org: behaviour on duplicates not verified. If a reply is lost after the file was accepted, the retry
  can create a duplicate import there (easy to delete).

## Real-time rate limit

While roaming, live posts go every 5 s (`API_SEND_SECONDS_DELAY_ROAMING`). tt.safecast.org answers
`429 too many measurements: maximum of 1 update every 5 min from a given location (+- 100m)` when the Zen is not
moving, for example when testing indoors with home 0,0. This does not affect the trip log file. Not implemented yet:
slow down posts when not moving, or back off after a 429.

## Testing without leaving the house

1. Run Real-time mode at home with a GPS fix.
2. Set the home latitude or longitude to 0 (web config; reboot if set from the SD file). After about 10 s a trip
   starts and `/rt-drive/` gets a file.
3. Restore the real home and range. After about 10 s the trip stops and, with WiFi, the file is uploaded and moves to
   `/rt-drive/uploaded/`.
4. Check api.safecast.org (imports list) and the simplemap admin uploads list, and delete the test uploads.

The first end-to-end test (2026-11-04) uploaded two trips (31 and 2 measurements) to both services; they appear on
api.safecast.org as "Processed (Not Approved)" until approved there.

## Notes on earlier work

- Branch `feature/dual-api-upload-url` (not merged) pointed a secondary upload URL at
  `https://simplemap.safecast.org/bgeigie_imports.json`. Simplemap has no such route: every unknown path returns the
  map web page with HTTP 200, so an upload there looks successful but imports nothing. Use `/upload` as above.
- The simplemap server needed no change: login with email + API key already works, and `/upload` accepts the session.

## Code map

- `handlers/rt_trip_handler.{h,cpp}`: state machine and upload sequencing
- `handlers/sd_logger.{h,cpp}`: new `rt_drive` log type
- `utils/log_upload.{h,cpp}`: api.safecast.org and simplemap upload
- `screens/fixed_mode.cpp`: switches the handler on/off with RT mode
- `workers/local_storage.*`, `utils/sd_wrapper.cpp`, `utils/http_pages.cpp`, `workers/configuration_server.cpp`:
  simplemap email setting
- `user_config.h`: folders, `RT_TRIP_DEBOUNCE_MS`, `RT_UPLOAD_RETRY_MS`, endpoints
