#ifndef BGEIGIEZEN_RT_TRIP_HANDLER_H_
#define BGEIGIEZEN_RT_TRIP_HANDLER_H_

#include <Handler.hpp>

#include "handlers/sd_logger.h"
#include "user_config.h"
#include "workers/local_storage.h"
#include "workers/log_aggregator.h"

/**
 * Real-time mode trip handling. Runs on fresh aggregated data only (no polling) and
 * moves through these states:
 *
 *   e_trip_home      at home, nothing pending
 *   e_trip_recording left the home fence: the rt-drive SD logger is active
 *   e_trip_upload    trip(s) waiting: /rt-drive/*.log -> api.safecast.org -> /rt-drive/api-done/
 *                    -> simplemap.safecast.org -> /rt-drive/uploaded/
 */
class RtTripHandler : public Handler {
 public:
  enum TripState {
    e_trip_home,
    e_trip_recording,
    e_trip_upload,
  };

  enum TripStatus {
    e_trip_status_upload_failed = e_handler_error + 1,
  };

  RtTripHandler(LocalStorage& config, SdLogger& logger);
  virtual ~RtTripHandler() = default;

  TripState trip_state() const { return _state; }

 protected:
  bool activate(bool retry) override;
  void deactivate() override;
  int8_t handle_produced_work(const worker_map_t& workers) override;

 private:
  void enter_state(TripState state);
  /** True when the roaming/home condition has been stable for RT_TRIP_DEBOUNCE_MS */
  bool track_roaming(const DataLine& data);
  int8_t try_upload();
  void write_upload_log(const char* service, const char* name, int code, const char* id);

  LocalStorage& _config;
  SdLogger& _logger;
  char _timestamp[20] = "";
  char _last_failed[LOG_FILENAME_SIZE + 8] = "";  // "<path> <http code>" of the last failure written to the upload log
  TripState _state = e_trip_home;
  bool _roaming = false;
  uint32_t _roaming_since = 0;
  uint32_t _last_upload_attempt = 0;
};

#endif //BGEIGIEZEN_RT_TRIP_HANDLER_H_
