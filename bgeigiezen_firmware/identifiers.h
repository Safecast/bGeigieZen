#ifndef BGEIGIEZEN_IDENTIFIERS_H_
#define BGEIGIEZEN_IDENTIFIERS_H_

/**
 * The worker identifiers in the system
 */
enum DataWorkers {
  k_worker_gps_connector = 0,
  k_worker_battery_indicator,
  k_worker_gm_sensor,
  k_worker_shake_detector,
  k_worker_button_A,
  k_worker_button_B,
  k_worker_button_C,
  k_worker_controller_state,
};

/**
 * The handler identifiers in the system
 */
enum DataHandlers {
  k_handler_controller_handler = 0,
  k_handler_bluetooth_reporter,
  k_handler_api_reporter,
};

#endif //BGEIGIEZEN_IDENTIFIERS_H_
