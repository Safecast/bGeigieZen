#ifndef BGEIGIEZEN_IDENTIFIERS_H_
#define BGEIGIEZEN_IDENTIFIERS_H_

/**
 * The worker identifiers in the system
 */
enum DataWorkers {
  k_worker_button_1 = 0,
  k_worker_button_2,
  k_worker_button_3,
  k_worker_screen_touch, // On M5Core black this is the same as button 1
  k_worker_gm_sensor,
  k_worker_battery_indicator,
  k_worker_gps_connector,
  k_worker_navsat_collector,
  k_worker_rtc_connector,
  k_worker_log_aggregator,
  k_worker_shake_detector,
  k_worker_device_state,
  k_worker_local_storage,
  k_worker_config_server,
};

/**
 * The handler identifiers in the system
 */
enum DataHandlers {
  k_handler_journal_logger = 0,
  k_handler_drive_logger,
  k_handler_survey_logger,
  k_handler_bluetooth_reporter,
  k_handler_api_reporter,
};

#endif //BGEIGIEZEN_IDENTIFIERS_H_
