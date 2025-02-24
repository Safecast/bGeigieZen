#ifndef BGEIGIEZEN_DEBUG_LOGGER_H_
#define BGEIGIEZEN_DEBUG_LOGGER_H_

#include <Handler.hpp>

#include "workers/local_storage.h"
#include "user_config.h"
#include "workers/log_aggregator.h"
#include "workers/navsat_collector.h"

/**
 * Debug log base, extend with write_line
 */
class BaseDebugLogger : public Handler {
 public:
  explicit BaseDebugLogger(LocalStorage& config, const char* logging_name);
  virtual ~BaseDebugLogger() = default;

 protected:
  bool activate(bool retry) override;
  virtual bool can_activate() {return true;};
  void deactivate() override;
  int8_t handle_produced_work(const worker_map_t& workers) final;

  virtual void write_header_lines() {};
  virtual bool write_line(const worker_map_t & workers) = 0;

  LocalStorage& _config;

  char _logging_name[20];
  char _logging_to[LOG_FILENAME_SIZE];
  bool _is_temp;
  uint32_t _total;

};


class GpsDebugLogger : public BaseDebugLogger {
 public:
  explicit GpsDebugLogger(LocalStorage& config, TeenyUbloxConnect& _gnss) : BaseDebugLogger(config, "gps"), gnss(_gnss) {};

 protected:
  bool can_activate() override;
  void write_header_lines() override;
  bool write_line(const worker_map_t& workers) override;

  TeenyUbloxConnect& gnss;
};


#endif //BGEIGIEZEN_DEBUG_LOGGER_H_
