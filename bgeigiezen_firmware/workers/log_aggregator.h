#ifndef WORKERS_LOG_AGGREGATOR_H
#define WORKERS_LOG_AGGREGATOR_H

#include "local_storage.h"
#include "user_config.h"
#include <Handler.hpp>

/**
 * Data collection
 */
struct DataLine {
  bool gps_valid = false;
  bool gm_valid = false;
  char log_string[LINE_BUFFER_SIZE] = "";

  bool valid() const {
    return (gps_valid && dop_valid || gps_fallback) && gm_valid;
  }

  uint32_t cpm = 0;  // Changed from uint16_t to handle high frequency inputs
  char timestamp[20] = "";
  double latitude = 0;
  double longitude = 0;
  double altitude = 0;
  double distance = 0;
  bool dop_valid = false;
  bool in_fixed_range = false;
  bool gps_fallback = false;  // GPS lost >10min while at home: send with last known location
};

/**
 * Log aggregator listens to sensor data from GPS and Geigie counter, and
 * creates single, complete readings to log every 5 seconds
 * Used by SD logger, API connector, BT connector
 */
class LogAggregator : public ProcessWorker<DataLine> {
 public:
  explicit LogAggregator(LocalStorage& settings);

 protected:

  int8_t produce_data(const WorkerMap& workers) override;

 private:
  LocalStorage& _settings;
  double _last_latitude;
  double _last_longitude;
  bool _last_in_fixed_range = false;
  uint32_t _last_valid_ms = 0;
  time_t _last_valid_time = 0;
};

#endif //WORKERS_LOG_AGGREGATOR_H
