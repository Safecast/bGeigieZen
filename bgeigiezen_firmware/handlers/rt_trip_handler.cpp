#include "rt_trip_handler.h"

#include <SD.h>
#include <WiFi.h>
#include <esp_heap_caps.h>
#include <mbedtls/platform.h>

#include "identifiers.h"
#include "utils/log_upload.h"
#include "utils/sd_wrapper.h"

// Core2 has too little internal RAM left for a TLS handshake (BIGNUM alloc failed): prefer PSRAM for mbedtls
static void* tls_calloc(size_t n, size_t size) {
  void* p = heap_caps_calloc(n, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  return p ? p : heap_caps_calloc(n, size, MALLOC_CAP_8BIT);
}
static void tls_free(void* p) {
  heap_caps_free(p);
}

RtTripHandler::RtTripHandler(LocalStorage& config, SdLogger& logger) : Handler(), _config(config), _logger(logger) {
}

void RtTripHandler::enter_state(TripState state) {
  _state = state;
  if (state == e_trip_upload) {
    memset(_stage_failed_at, 0, sizeof(_stage_failed_at));
  }
}

bool RtTripHandler::activate(bool) {
  _roaming = false;
  _roaming_since = millis();
  _last_fix_ms = millis();
  // Trips left on the SD card (e.g. power loss) are picked up by the upload state
  enter_state(SD.exists(RT_DRIVE_LOG_DIRECTORY) ? e_trip_upload : e_trip_home);
  return true;
}

void RtTripHandler::deactivate() {
  _logger.set_active(false);
  _state = e_trip_home;
}

bool RtTripHandler::track_roaming(const DataLine& data) {
  // No fix keeps the current condition; only a real fix can change it
  if (data.gps_valid && !data.gps_fallback) {
    _last_fix_ms = millis();
    if ((!data.in_fixed_range) != _roaming) {
      _roaming = !data.in_fixed_range;
      _roaming_since = millis();
    }
  } else if (_roaming && millis() - _last_fix_ms > GPS_LOST_FALLBACK_MS) {
    // No fix for 10+ min while roaming: the last fix was outside the home range (e.g. just before arriving), so
    // the aggregator never falls back. Assume home, otherwise the trip is never closed and never uploaded.
    _roaming = false;
    _roaming_since = 0;  // already debounced by the long outage
  }
  return millis() - _roaming_since > RT_TRIP_DEBOUNCE_MS;
}

int8_t RtTripHandler::handle_produced_work(const worker_map_t& workers) {
  const auto& aggregator = workers.worker<LogAggregator>(k_worker_log_aggregator);
  if (!aggregator->is_fresh()) {
    return e_handler_idle;
  }
  strncpy(_timestamp, aggregator->get_data().timestamp, sizeof(_timestamp) - 1);
  const bool stable = track_roaming(aggregator->get_data());

  switch (_state) {
    case e_trip_home:
      if (_roaming && stable && _logger.set_active(true)) {
        M5_LOGD("Left home, recording rt-drive trip");
        enter_state(e_trip_recording);
      }
      break;
    case e_trip_recording:
      if (!_logger.active()) {
        // SD card removed / logger stopped by the controller
        enter_state(e_trip_upload);
      } else if (!_roaming && stable) {
        M5_LOGD("Back home, trip recording stopped");
        _logger.set_active(false);
        enter_state(e_trip_upload);
      }
      break;
    case e_trip_upload:
      if (_roaming && stable) {
        // Left again before everything was uploaded: record the next trip first
        if (_logger.set_active(true)) {
          enter_state(e_trip_recording);
        }
        break;
      }
      return try_upload();
  }
  return e_handler_idle;
}

/**
 * One line per result in /rt-drive/uploads.log:
 * 2026-11-04T14:11:02Z api 2026-11-04_1403.log http=201 id=74020
 */
void RtTripHandler::write_upload_log(const char* service, const char* name, int code, const char* id) {
  char line[160];
  snprintf(line, sizeof(line), "%s %s %s http=%d%s%s", _timestamp, service, name, code, id[0] ? " id=" : "", id);
  SDInterface::i().log_println(RT_DRIVE_UPLOAD_LOG, line);
}

int8_t RtTripHandler::try_upload() {
  if (!WiFi.isConnected() || !SDInterface::i().ready()) {
    return e_handler_idle;
  }
  if (xTaskGetHandle("api_send") != nullptr) {
    return e_handler_idle;  // never run two TLS sessions at once (heap)
  }

  // API and simplemap are independent: each stage has its own retry timer, so a failing endpoint
  // (or file) never blocks the other one. API goes first, simplemap takes over when the API fails.
  struct Stage { const char* from; const char* to; bool simplemap; };
  const Stage stages[STAGES] = {
    {RT_DRIVE_LOG_DIRECTORY, RT_DRIVE_API_DONE_DIRECTORY, false},
    {RT_DRIVE_LOG_DIRECTORY, RT_DRIVE_MAP_DONE_DIRECTORY, true},
    {RT_DRIVE_API_DONE_DIRECTORY, RT_DRIVE_UPLOADED_DIRECTORY, true},
    {RT_DRIVE_MAP_DONE_DIRECTORY, RT_DRIVE_UPLOADED_DIRECTORY, false},
  };
  const char* history = strrchr(RT_DRIVE_UPLOAD_LOG, '/') + 1;

  bool waiting = false;  // a stage has files but is in its retry wait
  for (int i = 0; i < STAGES; i++) {
    const auto& st = stages[i];
    if (!SD.exists(st.from)) continue;
    // Pick the first trip log, preferring one that did not fail last time
    char name[64] = "";
    char failed_name[64] = "";
    File dir = SD.open(st.from);
    if (dir && dir.isDirectory()) {
      for (File f = dir.openNextFile(); f && !name[0]; f = dir.openNextFile()) {
        if (f.isDirectory()) continue;
        const char* n = strrchr(f.name(), '/');
        n = n ? n + 1 : f.name();
        const size_t len = strlen(n);
        // Only trip logs: skip the upload history (uploads.log) and anything that is not a .log
        if (strcmp(n, history) == 0 || len < 4 || len >= sizeof(name) || strcmp(n + len - 4, ".log") != 0) continue;
        if (strcmp(n, _stage_failed_name[i]) == 0) {
          snprintf(failed_name, sizeof(failed_name), "%s", n);
        } else {
          snprintf(name, sizeof(name), "%s", n);
        }
      }
    }
    if (dir) dir.close();
    if (!name[0]) snprintf(name, sizeof(name), "%s", failed_name);
    if (!name[0]) continue;

    if (_stage_failed_at[i] && millis() - _stage_failed_at[i] < RT_UPLOAD_RETRY_MS) {
      waiting = true;
      continue;  // this stage is waiting, try the next one
    }

    // Upload in a background task so the UI never blocks on the network
    snprintf(_task_path, sizeof(_task_path), "%s/%s", st.from, name);
    snprintf(_task_name, sizeof(_task_name), "%s", name);
    _task_to = st.to;
    _task_simplemap = st.simplemap;
    _task_stage = i;
    return start_task("rt_upload", 20480, 1);
  }

  if (!waiting) {
    // Nothing left to upload
    enter_state(e_trip_home);
  }
  return e_handler_idle;
}

int8_t RtTripHandler::handle_async() {
  const char* service = _task_simplemap ? "simplemap" : "api";
  M5_LOGD("Uploading trip log '%s' to %s (heap free=%u maxblk=%u psram=%u)", _task_path, service,
          (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMaxAllocHeap(), (unsigned)ESP.getFreePsram());
  char id[16] = "";
  int code;
  mbedtls_platform_set_calloc_free(tls_calloc, tls_free);
  if (_task_simplemap) {
    code = upload_log_file_simplemap(_task_path, _task_name, _config, id, sizeof(id));
  } else {
    uint32_t import_id = 0;
    code = upload_log_file(_task_path, _task_name, _config, &import_id);
    if (import_id) snprintf(id, sizeof(id), "%u", import_id);
  }
  if (code < 200 || code >= 300) {
    // Log a failure once per file/result, not on every 60 s retry
    char key[sizeof(_last_failed)];
    snprintf(key, sizeof(key), "%s %d", _task_path, code);
    if (strcmp(key, _last_failed) != 0) {
      strcpy(_last_failed, key);
      write_upload_log(service, _task_name, code, "");
    }
    _stage_failed_at[_task_stage] = millis() ? millis() : 1;
    snprintf(_stage_failed_name[_task_stage], sizeof(_stage_failed_name[0]), "%s", _task_name);
    return e_trip_status_upload_failed;  // this stage retries after RT_UPLOAD_RETRY_MS
  }
  write_upload_log(service, _task_name, code, id);
  char done[LOG_FILENAME_SIZE];
  snprintf(done, sizeof(done), "%s/%s", _task_to, _task_name);
  SDInterface::i().rename_log(_task_path, done);
  _stage_failed_at[_task_stage] = 0;
  _stage_failed_name[_task_stage][0] = '\0';
  return e_handler_data_handled;
}
