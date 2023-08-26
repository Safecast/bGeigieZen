#include "log_aggregator.h"
#include "gm_sensor.h"
#include "gps_connector.h"
#include "battery_indicator.h"
#include "identifiers.h"

LogAggregator::LogAggregator() : Worker<DataLine>(4000), Handler() {}

int8_t LogAggregator::produce_data() {
  return 0;
}

int8_t LogAggregator::handle_produced_work(const WorkerMap& workers) {
  const auto& gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);
  const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);
  const auto& battery = workers.worker<BatteryIndicator>(k_worker_battery_indicator);

  // TODO

  return e_handler_idle;
}
