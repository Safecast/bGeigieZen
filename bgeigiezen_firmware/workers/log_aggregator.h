#ifndef WORKERS_LOG_AGGREGATOR_H
#define WORKERS_LOG_AGGREGATOR_H

#include "handlers/local_storage.h"
#include <Handler.hpp>

/**
 * Data collection
 */
struct DataLine {
  bool valid;
  char log_string[100];

  uint16_t cpm;
  char timestamp[20];
  double longitude;
  double latitude;
  double altitude;
  bool in_fixed_range;
};

/**
 * Log aggregator listens to sensor data from GPS and Geigie counter, and
 * creates single, complete readings to log every 5 seconds
 * Used by SD logger, API connector, BT connector
 */
class LogAggregator : public ProcessWorker<DataLine> {
 public:
  LogAggregator(LocalStorage& settings);

 protected:

  int8_t produce_data(const WorkerMap& workers) override;

 private:
  LocalStorage& _settings;
};

#endif //WORKERS_LOG_AGGREGATOR_H
