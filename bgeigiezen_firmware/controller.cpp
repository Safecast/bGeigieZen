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
  set_worker_active(k_worker_local_storage, true);
  set_worker_active(k_worker_rtc_connector, true);
  set_worker_active(k_worker_battery_indicator, true);
  set_worker_active(k_worker_gps_connector, true);
  set_worker_active(k_worker_gm_sensor, true);
  set_worker_active(k_worker_log_aggregator, true);
//  set_worker_active(k_worker_shake_detector, true);
  set_worker_active(k_worker_button_3, true);
  set_worker_active(k_worker_button_2, true);
  set_worker_active(k_worker_button_1, true);
  set_worker_active(k_worker_device_state, true);
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
    }
  } else if (!SDInterface::i().ready()) {
    data.sd_card_status = SDInterface::SdStatus::e_sd_config_status_not_ready;
    _status = e_worker_data_read;
    // Stop all loggers to sd card
    set_handler_active(k_handler_journal_logger, false);
    set_handler_active(k_handler_drive_logger, false);
    set_handler_active(k_handler_survey_logger, false);
  } else if (data.sd_card_status != SDInterface::i().status()) {
    // SD status changed,
    data.sd_card_status = SDInterface::i().status();
    _status = e_worker_data_read;
  }

  if (_status == e_worker_data_read && _settings.get_device_id() && data.sd_card_status == SDInterface::e_sd_config_status_ok) {
    // start journal logger (if not active yet)
    set_handler_active(k_handler_journal_logger, true);
  }

  return _status;
}

void Controller::create_dummy_settings() {
  _settings.set_device_id(1, true);
  SDInterface::i().write_safezen_file_from_settings(_settings);
}

bool Controller::load_sd_config() {
  return SDInterface::i().read_safezen_file_to_settings(_settings);
}

bool Controller::write_sd_config() {
  return SDInterface::i().write_safezen_file_from_settings(_settings);
}

void Controller::reset_all() {
  _settings.reset_defaults();
  DeviceUtils::shutdown(true);
}
