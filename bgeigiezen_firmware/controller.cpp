#include "controller.h"
#include "identifiers.h"
#include "debugger.h"
#include "utils/sd_wrapper.h"

#include "utils/device_utils.h"

Controller::Controller() : Aggregator(),
                           Worker<DeviceState>({
                             false,
                             DeviceState::SDAvailability::e_sd_not_set,
                             DeviceState::Mode::e_mode_not_set
                           }),
                           _initialized(false) {
}

void Controller::initialize() {
  Activatable::initialize();
}

void Controller::start_default_workers() {

  // TODO: load sd_config into storage

  set_worker_active(k_worker_battery_indicator, true);
  set_worker_active(k_worker_gps_connector, true);
  set_worker_active(k_worker_gm_sensor, true);
//  set_worker_active(k_worker_shake_detector, true);
  set_worker_active(k_worker_button_3, true);
  set_worker_active(k_worker_button_2, true);
  set_worker_active(k_worker_button_1, true);
  set_worker_active(k_worker_controller_state, true);

  set_handler_active(k_handler_log_aggregator, true);


}

void Controller::reset_system() {
//  _config.reset_defaults();
  DeviceUtils::shutdown(true);
}

int8_t Controller::produce_data() {
  auto _status = e_worker_idle;

  if (_initialized && !data.initialized) {
    data.initialized = true;
    _status = e_worker_data_read;
  }

  if (SDInterface::i().ready()) {

  }
  return _status;
}
