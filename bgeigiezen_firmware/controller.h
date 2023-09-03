#ifndef BGEIGIEZEN_CONTROLLER_H_
#define BGEIGIEZEN_CONTROLLER_H_

#include <Handler.hpp>
#include <Aggregator.hpp>

struct DeviceState {
  enum SDAvailability {
    e_sd_not_set,
    e_sd_unavailable,
    e_sd_no_conf,
    e_sd_invalid_conf,
    e_sd_ready,
  };
  enum Mode {
    e_mode_not_set,
    e_mode_simple,
    e_mode_advanced,
  };

  bool initialized;
  SDAvailability sd_card_available;
  Mode advanced_mode;

};


/**
 * Main controller for the system, implements state machine to run
 */
class Controller : public Aggregator, public Worker<DeviceState> {
 public:
  Controller();
  virtual ~Controller() = default;

  /**
   *
   */
  void initialize() override;

  /**
   * Starts default workers and handlers, loads SD
   */
  void start_default_workers();

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

};

#endif // BGEIGIEZEN_CONTROLLER_H_
