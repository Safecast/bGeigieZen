#ifndef BGEIGIEZEN_DEBUG_LOGGER_H_
#define BGEIGIEZEN_DEBUG_LOGGER_H_

#include <Handler.hpp>

#include "workers/local_storage.h"
#include "user_config.h"
#include "workers/log_aggregator.h"

/**
 * Debug log base, extend with write_line
 */
class BaseDebugLogger : public Handler {
 public:
  explicit BaseDebugLogger(LocalStorage& config, const char* logging_name, const char* line_format_info);
  virtual ~BaseDebugLogger() = default;

 protected:
  bool activate(bool retry) override;
  void deactivate() override;
  int8_t handle_produced_work(const worker_map_t& workers) final;

  virtual bool write_line(const worker_map_t & workers) = 0;

  LocalStorage& _config;

  char _logging_name[20];
  char _line_format_info[120];
  char _logging_to[LOG_FILENAME_SIZE];
  bool _is_temp;
  uint32_t _total;

};


class GpsDebugLogger : public BaseDebugLogger {
 public:
  explicit GpsDebugLogger(LocalStorage& config) : BaseDebugLogger(config, "gps", "# hAcc,vAcc,velN,velE,velD,gSpeed,headMot,sAcc,headAcc,invalidLlh") {};

 protected:
  bool write_line(const worker_map_t& workers) override;
};


#endif //BGEIGIEZEN_DEBUG_LOGGER_H_
