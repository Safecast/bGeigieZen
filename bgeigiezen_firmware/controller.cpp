#include "controller_states.h"
#include "controller.h"
#include "identifiers.h"
#include "debugger.h"
#include "zen_button.h"

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif


Controller::Controller() :
    Context(),
    Aggregator(),
    Handler(),
    Worker<SystemStateId>(k_state_InitializeDeviceState),
    _state_changed(false) {
}

void Controller::setup_state_machine() {
  set_state(new InitializeDeviceState(*this));
}

void Controller::run() {
  Context::run();
  Aggregator::run();
}

void Controller::initialize() {
  //
  schedule_event(Event_enum::e_c_controller_initialized);
}

void Controller::shutdown(bool reboot) {
  if (reboot) {
    DEBUG_PRINTLN("\n Reboot system...\n");
    ESP.restart();
  } else {
    DEBUG_PRINTLN("\n Shutdown system...\n");
#ifdef M5_CORE2
    M5.shutdown();
#elif M5_BASIC
    M5.Power.powerOFF();
#endif
  }
}

void Controller::reset_system() {
//  _config.reset_defaults();
  shutdown(true);
}

int8_t Controller::handle_produced_work(const WorkerMap& workers) {
  const auto& buttonA = workers.worker<ZenButton>(k_worker_button_A);
  if(buttonA->is_fresh() && buttonA->get_data()) {
    DEBUG_PRINTLN("button A was pressed");
    schedule_event(Event_enum::e_c_button_A_pressed);
  }
  const auto& buttonB = workers.worker<ZenButton>(k_worker_button_B);
  if(buttonB->is_fresh() && buttonB->get_data()) {
    DEBUG_PRINTLN("button B was pressed");
    schedule_event(Event_enum::e_c_button_B_pressed);
  }
  const auto& buttonC = workers.worker<ZenButton>(k_worker_button_C);
  if(buttonC->is_fresh() && buttonC->get_data()) {
    DEBUG_PRINTLN("button C was pressed");
    schedule_event(Event_enum::e_c_button_C_pressed);
  }
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
