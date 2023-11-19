#include "identifiers.h"
#include "sd_logger.h"

SdLogger::SdLogger(LocalStorage& config, const LogType log_type) : Handler(), _config(config), _log_type(log_type) {
  strcpy(_logging_to, "");
}

bool SdLogger::activate(bool) {
  // TODO
  return false;
}

void SdLogger::deactivate() {
  // TODO
}

int8_t SdLogger::handle_produced_work(const worker_map_t& workers) {
  const auto& logData = workers.worker<LogAggregator>(k_worker_log_aggregator);
  if(logData->is_fresh()) {
    // TODO: write
  }

  return e_handler_idle;
}
