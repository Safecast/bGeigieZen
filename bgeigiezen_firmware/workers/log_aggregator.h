#ifndef WORKERS_LOG_AGGREGATOR_H
#define WORKERS_LOG_AGGREGATOR_H

#include <Handler.hpp>

/**
 * Data collection
 */
struct DataLine {
  bool valid;
  uint16_t cpm;
  char timestamp[20];
  char date[20];
  float longitude;
  float latitude;
  float altitude;

  char log_string[100];
  bool in_fixed_range;
};

/**
 * Log aggregator listens to sensor data from GPS and Geigie counter, and
 * creates single, complete readings to log every 5 seconds
 * Used by SD logger, API connector, BT connector
 */
class LogAggregator : public Worker<DataLine>, public Handler {
 public:
  LogAggregator();

 protected:

  int8_t produce_data() override;
  int8_t handle_produced_work(const WorkerMap& workers) override;
};

#endif //WORKERS_LOG_AGGREGATOR_H
