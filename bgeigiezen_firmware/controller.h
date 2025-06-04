#ifndef BGEIGIEZEN_CONTROLLER_H_
#define BGEIGIEZEN_CONTROLLER_H_

#include <Arduino.h>
#include <Worker.hpp>
#include <Aggregator.hpp>
#include "workers/battery_indicator.h"
#include "workers/rtc_connector.h"
#include "workers/local_storage.h"
#include "handlers/battery_logger.h"
#include "identifiers.h"

struct DeviceState {
  enum Mode {
    e_mode_not_set,
    e_mode_simple,
    e_mode_advanced,
  };

  bool initialized;
  bool local_available;
  SDInterface::SdStatus sd_card_status;
  Mode mode;

};


/**
 * Main controller for the system, implements state machine to run
 */
class Controller : public ProcessWorker<DeviceState>, public Aggregator {
    friend class Aggregator;
    friend class PowerManager;
    protected:
        WorkerMap workers;  // Make workers accessible to derived classes
    public:
  Controller(LocalStorage& settings, TeenyUbloxConnect& gnss);
  virtual ~Controller() = default;

  /**
   *
   */
  void initialize() override;

  /**
   * Starts default workers and handlers, loads SD
   */
  void start_default_workers();

  /**
   * Load SD config into device memory
   */
  bool load_sd_config();

  /**
   * Writes device settings onto SD
   *
   */
  bool write_sd_config();

  /**
   * Clears device memory, clears settings on the SD card, clears log files
   */
  bool factory_reset();

  /**
   * get GNSS instance
   */
  TeenyUbloxConnect& get_gnss();

  /**
   * Reset memory settings and restart system
   */
  void reset_settings();

  const LocalStorage& get_settings() const;

  void set_battery_logger(BatteryLogger* logger);

  // Worker access methods
  const BatteryStatus& get_battery_status() const {
    return workers.worker<BatteryIndicator>(k_worker_battery_indicator)->get_data();
  }

  const RtcData& get_rtc_data() const {
    return workers.worker<DateTimeProvider>(k_worker_rtc_connector)->get_data();
  }

 private:
  /**
   * As a worker, the controller will notify of any state changes
   * @return
   */
  int8_t produce_data(const WorkerMap& workers) override;

  bool _initialized;
  LocalStorage& _settings;
  TeenyUbloxConnect& _gnss;
  BatteryLogger* _battery_logger_ptr;

};

#endif // BGEIGIEZEN_CONTROLLER_H_
