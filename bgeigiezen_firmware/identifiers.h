#ifndef BGEIGIEZEN_IDENTIFIERS_H_
#define BGEIGIEZEN_IDENTIFIERS_H_

/**
 * The worker identifiers in the system
 */
enum DataWorkers {
  k_worker_gps_connector = 0,
  k_worker_battery_indicator,
#ifdef M5_CORE2
  k_worker_rtc_connector,
#endif
  k_worker_gm_sensor,
  k_worker_log_aggregator,
  k_worker_shake_detector,
  k_worker_button_1,
  k_worker_button_2,
  k_worker_button_3,
  k_worker_controller_state
};

/**
 * The handler identifiers in the system
 */
enum DataHandlers {
  k_handler_log_aggregator = 0,
  k_handler_bluetooth_reporter,
  k_handler_api_reporter,
  k_handler_local_storage,
};

#endif //BGEIGIEZEN_IDENTIFIERS_H_
