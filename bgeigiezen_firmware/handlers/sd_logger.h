#ifndef BGEIGIEZEN_JOURNAL_LOGGER_H_
#define BGEIGIEZEN_JOURNAL_LOGGER_H_

#include <Handler.hpp>

#include "local_storage.h"
#include "user_config.h"
#include "workers/log_aggregator.h"

/**
 * Setups up bluetooth endpoint for the device, allowing it to send readings over bluetooth
 */
class SdLogger : public Handler {
 public:
  enum LogType {
    journal,
    survey,
    drive,
    error,
  };

  explicit SdLogger(LocalStorage& config, LogType type);
  virtual ~SdLogger() = default;

 protected:
  bool activate(bool retry) override;
  void deactivate() override;
  int8_t handle_produced_work(const worker_map_t& workers) override;

 private:
  const char* get_dir() const;

  LocalStorage& _config;
  LogType _log_type;

  char _logging_to[LOG_FILENAME_SIZE];
  bool _is_temp;

};

#endif //BGEIGIEZEN_JOURNAL_LOGGER_H_
