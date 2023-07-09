#include "controller_states.h"
#include "controller.h"
#include "identifiers.h"
#include "debugger.h"
#include "zen_button.h"
#include "sd_wrapper.h"

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

Controller::Controller() : Context(),
                           Aggregator(),
                           Handler(),
                           Worker<SystemStateId>(k_state_InitializeDeviceState),
                           _state_changed(false) {
}

void Controller::setup_state_machine() {
  set_state(InitializeDeviceState::i(*this));
}

void Controller::run() {
  Context::run();
  Aggregator::run();
}

void Controller::initialize_device() {
#ifdef M5_CORE2
#elif M5_BASIC
#endif
  if (!SDInterface::i().begin()) {
    schedule_event(e_c_no_sd_card);
  }
  bool sd_config = SDInterface::i().get_safecast_content();
  if (!sd_config) {
    schedule_event(e_c_empty_sd_card);
  }

  // TODO: load sd_config into storage

  set_worker_active(k_worker_battery_indicator, true);
  set_worker_active(k_worker_gps_connector, true);
//  set_worker_active(k_worker_shake_detector, true);
  set_worker_active(k_worker_button_A, true);
  set_worker_active(k_worker_button_B, true);
  set_worker_active(k_worker_button_C, true);
  set_worker_active(k_worker_controller_state, true);

  set_handler_active(k_handler_controller_handler, true);

  schedule_event(e_c_device_initialized);
  schedule_event(Event_enum::e_c_controller_initialized);
}

void Controller::handle_empty_sd() {
  // TODO: Check device ID in local storage
  if (false) {
    bool success = SDInterface::i().generate_safecast_txt(1234); // replace 1234 with id from storage
    if (success) {
      schedule_event(e_c_sd_generated);
    } else {
      schedule_event(e_c_no_sd_card);
    }
  } else {
    schedule_event(e_c_no_sd_card);
  }
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
  if (buttonA->is_fresh() && buttonA->get_data()) {
    DEBUG_PRINTLN("button A was pressed");
    schedule_event(Event_enum::e_c_button_A_pressed);
  }
  const auto& buttonB = workers.worker<ZenButton>(k_worker_button_B);
  if (buttonB->is_fresh() && buttonB->get_data()) {
    DEBUG_PRINTLN("button B was pressed");
    schedule_event(Event_enum::e_c_button_B_pressed);
  }
  const auto& buttonC = workers.worker<ZenButton>(k_worker_button_C);
  if (buttonC->is_fresh() && buttonC->get_data()) {
    DEBUG_PRINTLN("button C was pressed");
    schedule_event(Event_enum::e_c_button_C_pressed);
  }
  return e_handler_data_handled;
}

void Controller::set_state(AbstractState& state) {
  Context::set_state(state);
  _state_changed = true;
}

int8_t Controller::produce_data() {
  if (_state_changed) {
    _state_changed = false;
    data = static_cast<SystemStateId>(get_current_state()->get_state_id());
    return e_worker_data_read;
  }
  return e_worker_idle;
}
