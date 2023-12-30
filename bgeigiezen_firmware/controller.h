#ifndef BGEIGIEZEN_CONTROLLER_H_
#define BGEIGIEZEN_CONTROLLER_H_

#include "workers/local_storage.h"
#include "utils/sd_wrapper.h"
#include <Aggregator.hpp>
#include <Handler.hpp>

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
class Controller : public Aggregator, public Worker<DeviceState> {
 public:
  Controller(LocalStorage& settings);
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
   * Create dummy settings local and sd card
   */
  void create_dummy_settings();

  /**
   * Load SD config into device memory
   */
  bool load_sd_config();

  /**
   * Writes device settings onto SD
   */
  bool write_sd_config();

  /**
   * Reset memory settings and restart system
   */
  void reset_all();

 private:
  /**
   * As a worker, the controller will notify of any state changes
   * @return
   */
  int8_t produce_data() override;

  /**
   * Reset and restart the system
   */
  void reset_system();

  bool _initialized;
  LocalStorage& _settings;

};

#endif // BGEIGIEZEN_CONTROLLER_H_
