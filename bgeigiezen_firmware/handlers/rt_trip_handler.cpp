#include "rt_trip_handler.h"

#include <SD.h>
#include <WiFi.h>

#include "identifiers.h"
#include "utils/log_upload.h"
#include "utils/sd_wrapper.h"

RtTripHandler::RtTripHandler(LocalStorage& config, SdLogger& logger, SdLogger& journal) : Handler(), _config(config), _logger(logger), _journal(journal) {
}

void RtTripHandler::enter_state(TripState state) {
  _state = state;
  if (state == e_trip_upload) {
    _last_upload_attempt = 0;
  }
}

bool RtTripHandler::activate(bool) {
  _roaming = false;
  _roaming_since = millis();
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
  if (data.gps_valid && !data.gps_fallback && (!data.in_fixed_range) != _roaming) {
    _roaming = !data.in_fixed_range;
    _roaming_since = millis();
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

int8_t RtTripHandler::try_upload() {
  if (!WiFi.isConnected() || !SDInterface::i().ready()) {
    return e_handler_idle;
  }
  if (_last_upload_attempt && millis() - _last_upload_attempt < RT_UPLOAD_RETRY_MS) {
    return e_handler_idle;
  }

  // Stage 1: /rt-drive -> api.safecast.org -> /rt-drive/api-done
  // Stage 2: /rt-drive/api-done -> simplemap.safecast.org -> /rt-drive/uploaded
  // API first, so simplemap sees the file already exists there and doesn't forward a duplicate.
  struct Stage { const char* from; const char* to; bool simplemap; };
  const Stage stages[] = {
    {RT_DRIVE_LOG_DIRECTORY, RT_DRIVE_API_DONE_DIRECTORY, false},
    {RT_DRIVE_API_DONE_DIRECTORY, RT_DRIVE_UPLOADED_DIRECTORY, true},
  };

  for (const auto& st : stages) {
    if (!SD.exists(st.from)) continue;
    char path[LOG_FILENAME_SIZE] = "";
    char name[64] = "";
    File dir = SD.open(st.from);
    if (dir && dir.isDirectory()) {
      for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
        if (!f.isDirectory()) {
          const char* n = strrchr(f.name(), '/');
          snprintf(name, sizeof(name), "%s", n ? n + 1 : f.name());
          snprintf(path, sizeof(path), "%s/%s", st.from, name);
          f.close();
          break;
        }
      }
    }
    if (dir) dir.close();
    if (!path[0]) continue;

    _last_upload_attempt = millis();
    M5_LOGD("Uploading trip log '%s' to %s", path, st.simplemap ? "simplemap" : "api");
    int code = st.simplemap ? upload_log_file_simplemap(path, name, _config) : upload_log_file(path, name, _config);
    char note[LOG_FILENAME_SIZE + 80];
    snprintf(note, sizeof(note), "%s rt-trip upload %s %s http=%d", _timestamp, st.simplemap ? "simplemap" : "api", path, code);
    _journal.log_comment(note);
    if (code < 200 || code >= 300) {
      return e_trip_status_upload_failed;  // retried after RT_UPLOAD_RETRY_MS
    }
    char done[LOG_FILENAME_SIZE];
    snprintf(done, sizeof(done), "%s/%s", st.to, name);
    SDInterface::i().rename_log(path, done);
    _last_upload_attempt = 0;  // next file / stage on the next data event
    return e_handler_data_handled;
  }

  // Nothing left to upload
  enter_state(e_trip_home);
  return e_handler_idle;
}
