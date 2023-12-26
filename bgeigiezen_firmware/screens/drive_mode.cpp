#include "drive_mode.h"
#include "debugger.h"
#include "handlers/sd_logger.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/battery_indicator.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/rtc_connector.h"
#include "workers/zen_button.h"

DriveModeScreen DriveModeScreen_i;

DriveModeScreen::DriveModeScreen() : BaseScreen("Drive", true), _logging_available(false), _currently_logging(false), _logging_start(0), _logging_stop(0) {
  required_tube = true;
  required_gps = true;
  required_sd = true;
}

BaseScreen* DriveModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto log_button = workers.worker<ZenButton>(k_worker_button_1);
  if (_logging_available && log_button->is_fresh() && log_button->get_data().shortPress) {
    controller.set_handler_active(k_handler_drive_logger, !_currently_logging);
  }

  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return &MenuWindow_i;
  }

  // TODO: handle something like this
//  if (controller.is_fresh() && controller.get_data().sd_card_status == SDInterface::SdStatus::e_sd_config_status_config_different_id) {
//    return &SdMessageScreen_i;
//  }

  return nullptr;
}

void DriveModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  const auto& controller_data = workers.worker<Controller>(k_worker_device_state)->get_data();
  _logging_available = controller_data.local_available && controller_data.sd_card_status == SDInterface::e_sd_config_status_ok;

  bool currently_logging = handlers.handler<SdLogger>(k_handler_drive_logger)->active();
  if (_currently_logging && !currently_logging) {
    _logging_stop = millis();
    _logging_start = 0;
  } else if (!_currently_logging && currently_logging) {
    _logging_start = millis();
    _logging_stop = 0;
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
    // Display CPM
    M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    auto cpm_width = printIntFont(gm_sensor->get_data().cpm_comp, 0, 100, 7);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawString(" CPM        ", 0 + cpm_width, 105, 4); // Prints after cpm value
    M5.Lcd.drawString("        ", 0 + cpm_width, 105 - 26, 4); // Prints blanks after cpm value, above CPM text

    // Display uSv/h
    M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    auto ush_width = printFloatFont(gm_sensor->get_data().uSv, 4, 0, 140, 4);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawString(" uSv/h", 0 + ush_width, 140, 4); // Prints after ush value
  }

  // Display GPS data always, change colour if not fresh
  if (gps->is_fresh() || force) {
    // Print drive data
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    const auto distance_width = M5.Lcd.drawString("Distance :", 0, 157, 1); // Prints after ush value
    const auto heading_width = M5.Lcd.drawString("Heading  :", 0, 165, 1); // Prints after ush value
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    printFloatFont(0.0, 1, 0 + distance_width, 157, 1); // Prints after ush value
    switch (gps->get_data().heading) {
      case GnssData::UNKNOWN:
        M5.Lcd.drawString("Unknown", 0 + heading_width, 165, 1);
        break;
      case GnssData::NORTH:
        M5.Lcd.drawString("North", 0 + heading_width, 165, 1);
        break;
      case GnssData::NORTHEAST:
        M5.Lcd.drawString("Northeast", 0 + heading_width, 165, 1);
        break;
      case GnssData::EAST:
        M5.Lcd.drawString("East", 0 + heading_width, 165, 1);
        break;
      case GnssData::SOUTHEAST:
        M5.Lcd.drawString("Southeast", 0 + heading_width, 165, 1);
        break;
      case GnssData::SOUTH:
        M5.Lcd.drawString("South", 0 + heading_width, 165, 1);
        break;
      case GnssData::SOUTHWEST:
        M5.Lcd.drawString("Southwest", 0 + heading_width, 165, 1);
        break;
      case GnssData::WEST:
        M5.Lcd.drawString("West", 0 + heading_width, 165, 1);
        break;
      case GnssData::NORTHWEST:
        M5.Lcd.drawString("Northwest", 0 + heading_width, 165, 1);
        break;
    }
    M5.Lcd.drawString("Somewhere", 0 + heading_width, 165, 1); // Prints after ush value

    // Print location data
    M5.Lcd.setCursor(170, 150);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Latitude   :");
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%.6f  ", gps->get_data().latitude);
    M5.Lcd.setCursor(170, 159);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Longitude  :");
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%.6f  ", gps->get_data().longitude);
    M5.Lcd.setCursor(170, 168);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Altitude   :");
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%.2f  ", gps->get_data().altitudeMSL);
    M5.Lcd.setCursor(170, 177);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("DOP        :");
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%.2f  ", gps->get_data().pdop);
  }
}

void DriveModeScreen::leave_screen(Controller& controller) {
  // close logging to file
  controller.set_handler_active(k_handler_drive_logger, false);
}

const __FlashStringHelper* DriveModeScreen::get_status_message(const worker_map_t& workers, const handler_map_t& handlers) const {
  if (_logging_start && _logging_start + STATUS_MESSAGE_DURATION > millis()) {
    return F(" STARTED LOGGING, safe travels! ");
  }
  if (_logging_stop && _logging_stop + STATUS_MESSAGE_DURATION > millis()) {
    return F(" COMPLETED LOGGING DRIVE ");
  }
  return BaseScreen::get_status_message(workers, handlers);
}
