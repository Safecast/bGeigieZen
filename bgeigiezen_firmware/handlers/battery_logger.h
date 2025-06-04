#ifndef BGEIGIEZEN_BATTERY_LOGGER_H_
#define BGEIGIEZEN_BATTERY_LOGGER_H_

#include <Handler.hpp>
#include "utils/sd_wrapper.h"
#include "workers/battery_indicator.h"
#include "workers/local_storage.h"
#include "workers/rtc_connector.h"

class BatteryLogger : public Handler {
public:
    explicit BatteryLogger(LocalStorage& config);
    virtual ~BatteryLogger() = default;

    bool activate(bool retry) override;
    void deactivate() override;
    int8_t handle_produced_work(const worker_map_t& workers) override;
    void log_battery_level_before_shutdown(const worker_map_t& workers);
    void set_log_filename(const char* filename);

 private:
  LocalStorage& _config;
  int32_t _last_battery_percentage = -1;
  char _log_filename[256];

  bool create_log_file(uint16_t year, uint8_t month, uint8_t day, LocalStorage::OperationalMode mode, uint16_t device_id);
};

#endif // BGEIGIEZEN_BATTERY_LOGGER_H_ 