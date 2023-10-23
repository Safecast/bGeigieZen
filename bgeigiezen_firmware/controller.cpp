#include "controller.h"
#include "identifiers.h"
#include "debugger.h"
#include "utils/sd_wrapper.h"

#include "utils/device_utils.h"

Controller::Controller(LocalStorage& settings)
    : Aggregator(),
      Worker<DeviceState>({false, false, false, SDInterface::SdStatus::e_sd_config_status_not_ready, DeviceState::Mode::e_mode_not_set}, 1000),
      _initialized(false),
      _settings(settings) {
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
  set_worker_active(k_worker_device_state, true);

  set_handler_active(k_handler_log_aggregator, true);

}

void Controller::reset_system() {
  _settings.reset_defaults();
  DeviceUtils::shutdown(true);
}

int8_t Controller::produce_data() {
  auto _status = e_worker_idle;

  if (_initialized && !data.initialized) {
    data.initialized = true;
    _status = e_worker_data_read;
  }

  if (DeviceState::e_mode_not_set == data.mode) {
    data.mode = DeviceState::e_mode_advanced;
    _status = e_worker_data_read;
  }

  if (!!_settings.get_device_id() != data.local_available) {
    data.local_available = !!_settings.get_device_id();
    _status = e_worker_data_read;
  }

  if (data.sd_card_status == SDInterface::SdStatus::e_sd_config_status_not_ready) {
    if (SDInterface::i().begin()) {
      // SD is inserted
      data.sd_card_status = SDInterface::i().has_safezen_content(_settings.get_device_id());
      _status = e_worker_data_read;
    } else {
      data.sd_card_status = SDInterface::SdStatus::e_sd_config_status_not_ready;
        _status = e_worker_data_read;
    }

  }
  return _status;
}
