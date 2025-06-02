#include "drive_mode.h"
#include "handlers/sd_logger.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/battery_indicator.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/gps_platform_model.h"
#include "workers/zen_button.h"
#include <esp_wifi.h>
#include <WiFi.h>

DriveModeScreen DriveModeScreen_i;

DriveModeScreen::DriveModeScreen() : BaseScreen("Drive", true), _logging_available(false), _currently_logging(false), _distance_start(0), _gps_model_set(false) {
  required_tube = true;
  required_gps = true;
  required_sd = true;
}

BaseScreen* DriveModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  // If we're about to leave this screen and we've set the GPS model, restore it
  static bool leaving_screen = false;
  if (leaving_screen && _gps_model_set) {
    auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
    if (gps) {
      gps->setDynamicModel(_previous_gps_model);
      _gps_model_set = false;
    }
    leaving_screen = false;
  }

  auto log_button = workers.worker<ZenButton>(k_worker_button_1);
  if (_logging_available && log_button->is_fresh() && log_button->get_data().shortPress) {
    controller.set_handler_active(k_handler_drive_logger, !_currently_logging);
  }

  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    leaving_screen = true; // Set flag to restore GPS model on next handle_input call
    return &MenuWindow_i;
  }

//   TODO: handle something like this
//  if (controller.is_fresh() && controller.get_data().sd_card_status == SDInterface::e_) {
//    return &SdMessageScreen_i;
//  }

  return nullptr;
}

void DriveModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  // Set GPS to AUTOMOTIVE mode on first render
  static bool first_render = true;
  if (first_render) {
    auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
    if (gps) {
      // Save current dynamic model to restore when leaving
      _previous_gps_model = gps->getDynamicModel();
      
      // Set to AUTOMOTIVE mode and save to flash memory
      if (gps->setDynamicModel(DYNMODEL_AUTOMOTIVE, true)) {
        // No message displayed when setting GPS mode
        _gps_model_set = true; // Mark that we've changed the GPS model
      }
    }
    first_render = false;
  }

  const auto& controller_data = workers.worker<Controller>(k_worker_device_state)->get_data();
  const auto& log_aggregator = workers.worker<LogAggregator>(k_worker_log_aggregator);
  _logging_available = controller_data.local_available && SDInterface::i().status() == SDInterface::e_sd_config_status_ok;

  bool currently_logging = handlers.handler<SdLogger>(k_handler_drive_logger)->active();
  if (!_currently_logging && currently_logging) {
    set_status_message(F(" STARTED LOGGING DRIVE "));
    _distance_start = log_aggregator->get_data().distance;
  }
  if (_currently_logging && !currently_logging) {
    set_status_message(F(" STOPPED LOGGING DRIVE "));
  }
  _currently_logging = currently_logging;

  /// Menu
  if (_logging_available && _currently_logging) {
    // Is logging
    drawButton1("Stop driving");
  }
  else {
    drawButton1("Start driving", _logging_available ? e_button_default : e_button_disabled);
  }
  drawButton2("");
  drawButton3("Menu");

  /// Display drive data
  const auto& gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);
  const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);


  if (gm_sensor->is_fresh() || force) {
    // Display values
    const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);
    if (settings->get_cpm_usvh()) {
      // Display CPM big, usvh small
      M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      uint16_t cpm_width = printIntFont(gm_sensor->get_data().cpm_comp, 0, 100, &fonts::Font7);
      uint16_t ush_width = printFloatFont(gm_sensor->get_data().uSvh, 4, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (CPM uSv/h)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(cpm_width, 52, 320 - cpm_width, 27, LCD_COLOR_BACKGROUND); // Prints blanks after cpm value, above CPM text
      cpm_width += M5.Lcd.drawString(" CPM", cpm_width, 105, &fonts::Font4); // Prints after cpm value
      M5.Lcd.fillRect(cpm_width, 74, 320 - cpm_width, 26, LCD_COLOR_BACKGROUND); // Prints blanks after CPM text
      M5.Lcd.drawString(" uSv/h   ", 0 + ush_width, 140, &fonts::Font4); // Prints after ush value
    } else {
      M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      uint16_t ush_width = printFloatFont(gm_sensor->get_data().uSvh, 3, 0, 100, &fonts::Font7);
      uint16_t cpm_width = printIntFont(gm_sensor->get_data().cpm_comp, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (CPM uSv/h)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(ush_width, 52, 320 - ush_width, 27, LCD_COLOR_BACKGROUND); // Prints blanks after cpm value, above CPM text
      ush_width += M5.Lcd.drawString(" uSv/h", ush_width, 105, &fonts::Font4); // Prints after cpm value
      M5.Lcd.fillRect(ush_width, 74, 320 - ush_width, 26, LCD_COLOR_BACKGROUND); // Prints blanks after CPM text
      M5.Lcd.drawString(" CPM   ", 0 + cpm_width, 140, &fonts::Font4); // Prints after ush value
    }
  }

  // Display GPS data always, change colour if not fresh
  if (gps->is_fresh()  || force) {
    // Print drive data
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    const uint16_t distance_width = M5.Lcd.drawString("Distance :", 0, 157, &fonts::Font0); // Prints after ush value
    const uint16_t heading_width = M5.Lcd.drawString("Heading  :", 0, 165, &fonts::Font0); // Prints after ush value
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    printFloatFont(log_aggregator->get_data().distance - _distance_start, 1, 0 + distance_width, 157, &fonts::Font0); // Prints after ush value
    switch (gps->get_data().heading) {
      case GnssData::UNKNOWN:
        M5.Lcd.drawString("Unknown", 0 + heading_width, 165, &fonts::Font0);
        break;
      case GnssData::NORTH:
        M5.Lcd.drawString("North", 0 + heading_width, 165, &fonts::Font0);
        break;
      case GnssData::NORTHEAST:
        M5.Lcd.drawString("Northeast", 0 + heading_width, 165, &fonts::Font0);
        break;
      case GnssData::EAST:
        M5.Lcd.drawString("East", 0 + heading_width, 165, &fonts::Font0);
        break;
      case GnssData::SOUTHEAST:
        M5.Lcd.drawString("Southeast", 0 + heading_width, 165, &fonts::Font0);
        break;
      case GnssData::SOUTH:
        M5.Lcd.drawString("South", 0 + heading_width, 165, &fonts::Font0);
        break;
      case GnssData::SOUTHWEST:
        M5.Lcd.drawString("Southwest", 0 + heading_width, 165, &fonts::Font0);
        break;
      case GnssData::WEST:
        M5.Lcd.drawString("West", 0 + heading_width, 165, &fonts::Font0);
        break;
      case GnssData::NORTHWEST:
        M5.Lcd.drawString("Northwest", 0 + heading_width, 165, &fonts::Font0);
        break;
    }

    // Print location data
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
    M5.Lcd.setCursor(170, 168);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Altitude   :");
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.2f    ", gps->get_data().altitudeMSL);
    M5.Lcd.setCursor(170, 177);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("DOP        :");
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.2f    ", gps->get_data().pdop);
  }
}

void DriveModeScreen::enter_screen(Controller& controller) {
  BaseScreen::enter_screen(controller);
  
  if (!controller.get_settings().get_manual_logging()) {
    // Automatically start logging
    controller.set_handler_active(k_handler_drive_logger, true);
    set_status_message(F(" STARTED LOGGING DRIVE "));
  }
  controller.set_handler_active(k_handler_bluetooth_reporter, true);

  // --- WiFi Power Save Mode and TX Power ---
  // Set WiFi to power save mode and reduce TX power for Drive mode
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM); // Enable minimum modem power save
  esp_wifi_set_max_tx_power(15);      // Set TX power to 15 (units: 0.25 dBm, so 15 = 3.75 dBm)
  // ----------------------------------------

  // We'll set the GPS to AUTOMOTIVE mode in the first render call
  // when we have access to the worker map
  force_next_render(); // Force render to apply GPS settings
}

void DriveModeScreen::leave_screen(Controller& controller) {
  // close logging to file
  controller.set_handler_active(k_handler_drive_logger, false);
  controller.set_handler_active(k_handler_bluetooth_reporter, false);

  // --- Restore WiFi Power Settings ---
  esp_wifi_set_ps(WIFI_PS_NONE);      // Disable WiFi power save
  esp_wifi_set_max_tx_power(78);      // Restore TX power to max (78 * 0.25 = 19.5 dBm)
  // -----------------------------------

  // Restore previous GPS dynamic model
  // Note: We can't access the GPS connector directly from here
  // The GPS model will be restored in the handle_input method
  // when the leaving_screen flag is set
}
