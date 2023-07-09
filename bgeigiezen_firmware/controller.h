#ifndef BGEIGIEZEN_CONTROLLER_HPP
#define BGEIGIEZEN_CONTROLLER_HPP

#include <Handler.hpp>
#include <Aggregator.hpp>
#include <sm_context.h>

/**
 * System states
 */
enum SystemStateId {
  k_state_InitializeDeviceState,   // M5Stack startup
  k_state_NoSDCardState,           // No SD card found, end of the line for now
  k_state_EmptySDCardState,        // Empty SD card found, either restart or generate SAFECAST.txt
  k_state_PostInitializeState,     // Splash screen time
  k_state_ConfigurationModeState,  // Take measurements, display and upload data
  k_state_MeasurementModeState,    // Config server to direct connect and configure
  k_state_ResetState,              // Reset the system settings and restart
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
  void set_state(AbstractState& state) override;

 private:

  /**
   * Init device
   */
  void initialize_device();

  /**
   * Init device
   */
  void handle_empty_sd();

  /**
   * Reads buttons and other input workers to create events for the state machine
   * @return
   */
  int8_t handle_produced_work(const worker_map_t& workers) override;

  /**
   * As a worker, the controller will notify of any state changes
   * @return
   */
  int8_t produce_data() override;

  /**
   * Reset and restart the system
   */
  void shutdown(bool reboot = false);

  /**
   * Reset and restart the system
   */
  void reset_system();

  bool _state_changed;

  friend class InitializeDeviceState;
  friend class NoSDCardState;
  friend class EmptySDCardState;
  friend class PostInitializeState;
  friend class ConfigurationModeState;
  friend class MeasurementModeState;
  friend class ResetState;
};

#endif // BGEIGIEZEN_CONTROLLER_HPP
