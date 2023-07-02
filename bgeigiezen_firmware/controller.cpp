#include "controller_states.h"
#include "controller.h"
#include "identifiers.h"

#define BUTTON_LONG_PRESSED_MILLIS_THRESHOLD 4000

Controller::Controller() :
    Context(),
    Aggregator(),
    Handler(),
    Worker<SystemStateId>(k_state_InitializeHardwareState),
    _state_changed(false) {
}

void Controller::setup_state_machine() {
  set_state(new InitializeHardwareState(*this));
}

void Controller::run() {
  Context::run();
  Aggregator::run();
}

void Controller::initialize() {
  //
  schedule_event(Event_enum::e_c_controller_initialized);
}

void Controller::reset_system() {
//  _config.reset_defaults();
//  DEBUG_PRINTLN("\n RESTARTING ESP...\n");
  ESP.restart();
}

int8_t Controller::handle_produced_work(const WorkerMap& workers) {
  return e_handler_data_handled;
}

void Controller::set_state(AbstractState* state) {
  Context::set_state(state);
  _state_changed = true;
}

int8_t Controller::produce_data() {
  if(_state_changed) {
    _state_changed = false;
    data = static_cast<SystemStateId>(get_current_state()->get_state_id());
    return e_worker_data_read;
  }
  return e_worker_idle;
}
