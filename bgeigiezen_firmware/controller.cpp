#include "controller.h"
#include "identifiers.h"
#include "utils/sd_wrapper.h"
#include "utils/device_utils.h"
#include "workers/gm_sensor.h"
#include "workers/rtc_connector.h"
#include "workers/sound_manager.h"
#include "screens/default_entry_screen.h"
#include "screens/menu_window.h"
#include "workers/battery_indicator.h"
#include "workers/log_aggregator.h"
#include "workers/rtc_connector.h"
#include "workers/zen_button.h"
#include "handlers/sd_logger.h"
#include "handlers/bluetooth_reporter.h"
#include "handlers/api_connector.h"
#include "handlers/battery_logger.h"

Controller::Controller(LocalStorage& settings, TeenyUbloxConnect& gnss) 
    : ProcessWorker<DeviceState>({false, false, SDInterface::SdStatus::e_sd_config_status_not_ready, DeviceState::Mode::e_mode_not_set}, 1000),
      Aggregator(),
      _settings(settings),
      _gnss(gnss),
      _initialized(false) {
    _battery_logger_ptr = nullptr;
    // Initialize data with default values
    data.initialized = false;
    data.local_available = false;
    data.sd_card_status = SDInterface::SdStatus::e_sd_config_status_not_ready;
    data.mode = DeviceState::e_mode_not_set;
}

void Controller::initialize() {
    if (_initialized) return;

    // Start workers and handlers that should be active from boot
    start_default_workers();

    _initialized = true;
    M5_LOGD("Controller initialized.");
}

void Controller::start_default_workers() {
  set_worker_active(k_worker_button_3, true);
  set_worker_active(k_worker_button_2, true);
  set_worker_active(k_worker_button_1, true);
  set_worker_active(k_worker_local_storage, true);
  set_worker_active(k_worker_rtc_connector, true);
  set_worker_active(k_worker_battery_indicator, true);
  set_worker_active(k_worker_gm_sensor, true);
  set_worker_active(k_worker_gps_connector, true);
  set_worker_active(k_worker_navsat_collector, true);
  set_worker_active(k_worker_log_aggregator, true);
//  set_worker_active(k_worker_shake_detector, true);
  set_worker_active(k_worker_device_state, true);
  set_worker_active(k_worker_sound_manager, true);
  
  // We'll connect the SoundManager to the GeigerCounter after all workers are initialized
  // This will be done in the produce_data method
}

int8_t Controller::produce_data(const WorkerMap& workers) {
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
    set_handler_active(k_handler_flight_logger, false);
    set_handler_active(k_handler_gps_debug_logger, false);
    // Deactivate battery logger as well
    set_handler_active(k_handler_battery_logger, false);
  } else if (data.sd_card_status != SDInterface::i().status()) {
    // SD status changed,
    data.sd_card_status = SDInterface::i().status();
    _status = e_worker_data_read;
  }

  if (_status == e_worker_data_read && _settings.get_device_id() && data.sd_card_status == SDInterface::e_sd_config_status_ok) {
    // start journal logger (if not active yet)
    if (_settings.get_enable_journal()) {
      set_handler_active(k_handler_journal_logger, true);
    }
    set_handler_active(k_handler_gps_debug_logger, true);

    // Check if battery logger is active and file exists, if not, create file and activate logger
    if (_battery_logger_ptr && !_battery_logger_ptr->active()) {
        const auto& rtc = workers.worker<DateTimeProvider>(k_worker_rtc_connector);
        if (rtc && rtc->active() && rtc->get_data().valid) {
            const auto& rtc_data = rtc->get_data();
            char log_filename[256];
            sprintf(log_filename, "%s/%04d-%02d-%02d.csv", BATTERY_LOG_DIRECTORY, rtc_data.year, rtc_data.month, rtc_data.day);

            // Check if directory exists, create if not
            if (!SD.exists(BATTERY_LOG_DIRECTORY)) {
                SD.mkdir(BATTERY_LOG_DIRECTORY);
            }

            // Check if file already exists. If not, create and write header.
            if (!SD.exists(log_filename)) {
                M5_LOGD("Controller: Creating new battery log file %s with header.", log_filename);
                File logFile = SD.open(log_filename, FILE_WRITE);
                if (logFile) {
                    // Write header line
                    logFile.printf("# Device ID: %u\n", _settings.get_device_id());

                    const char* mode_str;
                    switch(_settings.get_last_mode()) {
                        case LocalStorage::e_operational_mode_drive: mode_str = "Drive"; break;
                        case LocalStorage::e_operational_mode_survey: mode_str = "Survey"; break;
                        case LocalStorage::e_operational_mode_fixed: mode_str = "Fixed"; break;
                        case LocalStorage::e_operational_mode_satellite: mode_str = "Satellite"; break;
                        case LocalStorage::e_operational_mode_flight: mode_str = "Flight"; break;
                        default: mode_str = "Unknown"; break;
                    }
                    logFile.printf("# Operational Mode: %s\n", mode_str);

                    logFile.println("Date,Time,BatteryLevel"); // CSV header
                    logFile.close();
                } else {
                    M5_LOGE("Controller: Failed to create battery log file %s", log_filename);
                }
            }

            // If the file now exists (either existed before or was just created successfully), activate the logger
            if (SD.exists(log_filename)) {
                _battery_logger_ptr->set_log_filename(log_filename);
                set_handler_active(k_handler_battery_logger, true);
                M5_LOGD("Controller: Battery logger activated for file: %s", log_filename);
            }
        }
    }
  }

  return _status;
}

bool Controller::load_sd_config() {
  return SDInterface::i().read_safezen_file_to_settings(_settings);
}

bool Controller::write_sd_config() {
  return SDInterface::i().write_safezen_file_from_settings(_settings);
}

bool Controller::factory_reset() {
  SDInterface::i().clear_all_logs();
//  SDInterface::i().write_safezen_file_from_settings(_settings, true);
  reset_settings();
  return true;
}

TeenyUbloxConnect& Controller::get_gnss() {
  return _gnss;
}

void Controller::reset_settings() {
  _settings.reset_defaults();
}

const LocalStorage& Controller::get_settings() const {
  return _settings;
}

void Controller::set_battery_logger(BatteryLogger* logger) {
    _battery_logger_ptr = logger;
    M5_LOGD("Controller: BatteryLogger pointer set.");
}
