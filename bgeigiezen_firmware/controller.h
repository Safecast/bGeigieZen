#ifndef BGEIGIEZEN_CONTROLLER_HPP
#define BGEIGIEZEN_CONTROLLER_HPP

#include <Handler.hpp>
#include <Aggregator.hpp>
#include <sm_context.h>


enum SystemStateId {
  k_state_InitializeHardwareState,  // M5Stack startup
  k_state_InitializeSoftwareState,  // Init software (workers, storage etc.)
  k_state_ConfigurationModeState,  // Take measurements, display and upload data
  k_state_MeasurementModeState,  // Config server to direct connect and configure
  k_state_ResetState,  // Reset the system settings and restart
};

/**
 * Main controller for the system, implements state machine to run
 */
class Controller : public Context, public Aggregator, public Handler, public Worker<SystemStateId> {
 public:

  Controller();
  virtual ~Controller() = default;

  /**
   * Set initial state for the state machine,
   */
  void setup_state_machine();

  /**
   * Override the context run to also run the reporter state machine
   */
  void run() override;

  /**
   * override set state from context, to flag worker that change has been made
   * @param state
   */
  void set_state(AbstractState* state) override;

 private:
  void initialize() override;
  int8_t handle_produced_work(const worker_map_t& workers) override;

  int8_t produce_data() override;

  /**
   * Reset and restart the system
   */
  void reset_system();

  bool _state_changed;

  friend class InitializeHardwareState;
  friend class InitializeSoftwareState;
  friend class ConfigurationModeState;
  friend class MeasurementModeState;
  friend class ResetState;
};

#endif //BGEIGIEZEN_CONTROLLER_HPP
