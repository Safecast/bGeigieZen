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
    return gps_valid && gm_valid && dop_valid;
  }

  uint16_t cpm = 0;
  char timestamp[20] = "";
  double latitude = 0;
  double longitude = 0;
  double altitude = 0;
  double distance = 0;
  bool dop_valid = false;
  bool in_fixed_range = false;

  // Power monitor
  float ibatt = 0.0;
  float vbatt = 0.0;
  float iboost = 0.0;
  float vboost = 0.0;
  float ibus = 0.0;
  float vbus = 0.0;
  uint8_t percentage = 0;
  bool isCharging = false;
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
};

#endif //WORKERS_LOG_AGGREGATOR_H
