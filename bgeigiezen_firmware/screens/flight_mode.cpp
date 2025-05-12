#include "flight_mode.h"
#include "handlers/sd_logger.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/battery_indicator.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/log_aggregator.h"
#include "workers/zen_button.h"

FlightModeScreen FlightModeScreen_i;

FlightModeScreen::FlightModeScreen() : BaseScreen("Cosmic", true), _logging_available(false), _currently_logging(false), _distance_start(0), _previous_gps_model(DYNMODEL_PORT) {
  required_tube = true;
  required_gps = true;
  required_wifi = false;  // WiFi is optional for Flight mode
  required_sd = true;
}

BaseScreen* FlightModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto log_button = workers.worker<ZenButton>(k_worker_button_1);
  if (_logging_available && log_button->is_fresh() && log_button->get_data().shortPress) {
    controller.set_handler_active(k_handler_flight_logger, !_currently_logging);
  }

  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    // Restore previous GPS dynamic platform model before returning to menu
    auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
    if (gps) {
      gps->setDynamicModel(_previous_gps_model);
      // No message displayed when leaving Flight mode
    }
    return &MenuWindow_i;
  }
  return nullptr;
}



void FlightModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  // Set GPS to AIR4 mode on first render
  static bool first_render = true;
  if (first_render) {
    auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
    if (gps) {
      // Save current dynamic model to restore when leaving
      _previous_gps_model = gps->getDynamicModel();
      
      // Set to AIR4 mode and save to flash memory
      if (gps->setDynamicModel(DYNMODEL_AIR4, true)) {
        // No message displayed when setting GPS mode
      }
    }
    first_render = false;
  }
  
  const auto& controller_data = workers.worker<Controller>(k_worker_device_state)->get_data();
  const auto& log_aggregator = workers.worker<LogAggregator>(k_worker_log_aggregator);
  _logging_available = controller_data.local_available && SDInterface::i().status() == SDInterface::e_sd_config_status_ok;

  bool currently_logging = handlers.handler<SdLogger>(k_handler_flight_logger)->active();
  if (!_currently_logging && currently_logging) {
    set_status_message(F(" STARTED LOGGING FLIGHT "));
    _distance_start = log_aggregator->get_data().distance;
  }
  if (_currently_logging && !currently_logging) {
    set_status_message(F(" STOPPED LOGGING FLIGHT "));
  }
  _currently_logging = currently_logging;

  // Button labels
  if (_logging_available && _currently_logging) {
    drawButton1("Stop flight");
  } else {
    drawButton1("Start flight", _logging_available ? e_button_default : e_button_disabled);
  }
  
  drawButton2("");
  drawButton3("Menu");

  // Display radiation data
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
  if (gps->is_fresh() || force) {
    // Print flight data
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    const uint16_t distance_width = M5.Lcd.drawString("Distance :", 0, 157, &fonts::Font0);
    const uint16_t heading_width = M5.Lcd.drawString("Heading  :", 0, 165, &fonts::Font0);
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    printFloatFont(log_aggregator->get_data().distance - _distance_start, 1, 0 + distance_width, 157, &fonts::Font0);
    
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

    // Print location data with emphasis on altitude and vertical speed
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
    
    // Highlight altitude for Flight mode
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
    M5.Lcd.setTextColor(LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);  // Use activity color for vertical speed too
    M5.Lcd.print("VERT SPEED :");
    
    // Check if we have valid velocity data
    if (gps->get_data().location_valid) {
      // Convert from mm/s to m/s and invert sign (velD is positive downward)
      float verticalSpeed = -gps->get_data().velD / 1000.0f;
      M5.Lcd.setTextColor(LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);
      M5.Lcd.printf("%0.1f m/s ", verticalSpeed);
    } else {
      M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      M5.Lcd.print("N/A      ");
    }
  }
}

void FlightModeScreen::enter_screen(Controller& controller) {
  // Start logging by default if manual logging is disabled
  if (!controller.get_settings().get_manual_logging()) {
    controller.set_handler_active(k_handler_flight_logger, true);
    _currently_logging = true;
    set_status_message(F(" STARTED LOGGING FLIGHT "));
  }
  
  // We'll set the GPS to AIR4 mode in the first render call
  // when we have access to the worker map
  force_next_render(); // Force render to apply GPS settings
  
  // Enable BLE for Flight mode, similar to Drive mode
  controller.set_handler_active(k_handler_bluetooth_reporter, true);
}

void FlightModeScreen::leave_screen(Controller& controller) {
  // Stop logging and BLE when leaving the screen
  controller.set_handler_active(k_handler_flight_logger, false);
  controller.set_handler_active(k_handler_bluetooth_reporter, false);
  _currently_logging = false;
  
  // We need to get the worker map from the handle_input method, so we'll set the model back in that method
  // or in the enter_screen method of the next screen
  // No message displayed when leaving Flight mode
}
