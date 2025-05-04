#include "air_mode.h"
#include "handlers/api_connector.h"
#include "handlers/sd_logger.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/log_aggregator.h"
#include "workers/zen_button.h"

AirModeScreen AirModeScreen_i;

AirModeScreen::AirModeScreen() : BaseScreen("Air Mode", true), _logging_available(false), _currently_logging(false), _transmitting_data(false), _wifi_enabled(false), _ble_enabled(false), _previous_gps_model(DYNMODEL_PORT) {
  required_tube = true;
  required_gps = true;
  required_sd = true;
  required_wifi = false;  // WiFi is optional for Air mode
}

BaseScreen* AirModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto log_button = workers.worker<ZenButton>(k_worker_button_1);
  if (_logging_available && log_button->is_fresh() && log_button->get_data().shortPress) {
    controller.set_handler_active(k_handler_survey_logger, !_currently_logging);
  }

  auto transmit_button = workers.worker<ZenButton>(k_worker_button_2);
  if (transmit_button->is_fresh() && transmit_button->get_data().shortPress) {
    // Toggle wireless transmission
    _wifi_enabled = !_wifi_enabled;
    _ble_enabled = !_ble_enabled;
    _transmitting_data = _wifi_enabled || _ble_enabled;
    
    // Update status message based on new state
    if (_wifi_enabled && _ble_enabled) {
      set_status_message(F(" WIFI AND BLE ENABLED "));
    } else if (_wifi_enabled) {
      set_status_message(F(" WIFI ENABLED, BLE DISABLED "));
    } else if (_ble_enabled) {
      set_status_message(F(" BLE ENABLED, WIFI DISABLED "));
    } else {
      set_status_message(F(" WIRELESS TRANSMISSION DISABLED "));
    }
    
    // Activate/deactivate handlers based on new state
    controller.set_handler_active(k_handler_api_reporter, _wifi_enabled);
    controller.set_handler_active(k_handler_bluetooth_reporter, _ble_enabled);
  }

  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    // Restore previous GPS dynamic platform model before returning to menu
    auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
    if (gps) {
      gps->setDynamicModel(_previous_gps_model);
      set_status_message(F(" LEAVING AIR MODE "));
    }
    return &MenuWindow_i;
  }
  return nullptr;
}

void AirModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  // Set GPS to AIR4 mode on first render
  static bool first_render = true;
  if (first_render) {
    auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
    if (gps) {
      // Save current dynamic model to restore when leaving
      _previous_gps_model = gps->getDynamicModel();
      
      // Set to AIR4 mode
      if (gps->setDynamicModel(DYNMODEL_AIR4)) {
        set_status_message(F(" AIR MODE - GPS SET TO AIRBORNE 4G "));
      }
    }
    first_render = false;
  }
  
  const auto& controller_data = workers.worker<Controller>(k_worker_device_state)->get_data();
  _logging_available = controller_data.local_available && SDInterface::i().status() == SDInterface::e_sd_config_status_ok;
  bool currently_logging = handlers.handler<SdLogger>(k_handler_survey_logger)->active();
  
  if (_currently_logging && !currently_logging) {
    set_status_message(F(" COMPLETED LOGGING AIR DATA "));
  } else if (!_currently_logging && currently_logging) {
    set_status_message(F(" STARTED LOGGING AIR DATA "));
  }
  _currently_logging = currently_logging;

  // Button labels
  if (_logging_available && _currently_logging) {
    drawButton1("Stop logging");
  } else {
    drawButton1("Start logging", _logging_available ? e_button_default : e_button_disabled);
  }
  
  if (_wifi_enabled || _ble_enabled) {
    drawButton2("Disable wireless");
  } else {
    drawButton2("Enable wireless");
  }
  
  drawButton3("Menu");

  // Display radiation data
  const auto& gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);
  const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);
  const auto& api_connector = handlers.handler<ApiConnector>(k_handler_api_reporter);

  if (gm_sensor->is_fresh() || force) {
    // Display values
    const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);
    if (settings->get_cpm_usvh()) {
      // Display CPM big, usvh small
      M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      uint16_t cpm_width = printIntFont(gm_sensor->get_data().cp5s, 0, 100, &fonts::Font7);
      uint16_t ush_width = printFloatFont(gm_sensor->get_data().uSvh_5sec, 4, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (CPM uSv/h)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(cpm_width, 52, 320 - cpm_width, 27, LCD_COLOR_BACKGROUND);
      cpm_width += M5.Lcd.drawString(" CP5S", cpm_width, 105, &fonts::Font4);
      M5.Lcd.fillRect(cpm_width, 74, 320 - cpm_width, 26, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString(" uSv/h   ", 0 + ush_width, 140, &fonts::Font4);
    } else {
      M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      uint16_t ush_width = printFloatFont(gm_sensor->get_data().uSvh_5sec, 3, 0, 100, &fonts::Font7);
      uint16_t cpm_width = printIntFont(gm_sensor->get_data().cp5s, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (CPM uSv/h)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(ush_width, 52, 320 - ush_width, 27, LCD_COLOR_BACKGROUND);
      ush_width += M5.Lcd.drawString(" uSv/h", ush_width, 105, &fonts::Font4);
      M5.Lcd.fillRect(ush_width, 74, 320 - ush_width, 26, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawString(" CP5S   ", 0 + cpm_width, 140, &fonts::Font4);
    }
  }

  // Display GPS and altitude data
  if (gps->is_fresh() || force) {
    // Print location data with emphasis on altitude
    M5.Lcd.setCursor(170, 150);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Latitude   :");
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.6f  ", gps->get_data().latitude);
    
    M5.Lcd.setCursor(170, 159);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Longitude  :");
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.6f  ", gps->get_data().longitude);
    
    // Highlight altitude for Air mode
    M5.Lcd.setCursor(170, 168);
    M5.Lcd.setTextColor(LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);  // Use activity color to highlight altitude
    M5.Lcd.print("ALTITUDE   :");
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_ACTIVITY : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.2f m   ", gps->get_data().altitudeMSL);
    
    M5.Lcd.setCursor(170, 177);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("DOP        :");
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.2f    ", gps->get_data().pdop);
    
    // Calculate and display vertical speed using velD
    M5.Lcd.setCursor(170, 186);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Vert. Speed:");
    
    // Check if we have valid velocity data
    if (gps->get_data().location_valid) {
      // Convert from mm/s to m/s and invert sign (velD is positive downward)
      float verticalSpeed = -gps->get_data().velD / 1000.0f;
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.printf("%0.1f m/s ", verticalSpeed);
    } else {
      M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      M5.Lcd.print("N/A      ");
    }
  }

  // Display transmission status
  if (_transmitting_data) {
    if (api_connector->get_status() == ApiConnector::e_handler_processing) {
      set_status_message(F(" SENDING DATA... "));
    } else if (api_connector->get_status() == ApiConnector::e_api_reporter_send_success) {
      set_status_message(F(" SENT DATA TO API! "));
    }
    
    // Display transmission count
    M5.Lcd.setCursor(0, 177);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("Transmissions: %d", api_connector->get_post_count());
  }
}

void AirModeScreen::enter_screen(Controller& controller) {
  // Start logging by default if manual logging is disabled
  if (!controller.get_settings().get_manual_logging()) {
    controller.set_handler_active(k_handler_survey_logger, true);
    _currently_logging = true;
  }
  
  // Ensure WiFi and BLE are disabled by default in Air mode
  _wifi_enabled = false;
  _ble_enabled = false;
  _transmitting_data = false;
  controller.set_handler_active(k_handler_api_reporter, false);
  controller.set_handler_active(k_handler_bluetooth_reporter, false);
  
  // We'll set the GPS to AIR4 mode in the first render call
  // when we have access to the worker map
  set_status_message(F(" AIR MODE - WIRELESS DISABLED "));
  force_next_render(); // Force render to apply GPS settings
}

void AirModeScreen::leave_screen(Controller& controller) {
  // Stop logging and transmitting when leaving the screen
  controller.set_handler_active(k_handler_survey_logger, false);
  controller.set_handler_active(k_handler_api_reporter, false);
  controller.set_handler_active(k_handler_bluetooth_reporter, false);
  _currently_logging = false;
  _wifi_enabled = false;
  _ble_enabled = false;
  _transmitting_data = false;
  
  // We need to get the worker map from the handle_input method, so we'll set the model back in that method
  // or in the enter_screen method of the next screen
  set_status_message(F(" LEAVING AIR MODE "));
}
