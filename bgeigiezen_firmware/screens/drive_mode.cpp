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

DriveModeScreen::DriveModeScreen() : BaseScreen("Drive", true), _log_available(false) {
}

BaseScreen* DriveModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  // TODO: handle log button

  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return MenuWindow::i();
  }
  return nullptr;
}

void DriveModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  _log_available = workers.worker<Controller>(k_worker_device_state)->get_data().sd_card_status == SDInterface::e_sd_config_status_ok;
  bool logger_active = handlers.handler<SdLogger>(k_handler_drive_logger)->get_active_state() == Handler::e_state_active;
  /// Menu
  if (_log_available && logger_active) {
    // Is logging
    drawButton1("Stop log");
  }
  else {
    drawButton1("Start log", _log_available ? e_button_default : e_button_disabled);
  }
  drawButton2("");
  drawButton3("Menu");

  /// Display drive data
  const auto& gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);
  const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);

  if (gm_sensor->get_data().valid) {
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  }
  else {
    M5.Lcd.setTextColor(LCD_COLOR_OLD, LCD_COLOR_BACKGROUND);
  }

  if (gm_sensor->is_fresh() || force) {

    // Display CPM
    auto cpm_width = printIntFont(gm_sensor->get_data().cpm_comp, 20, 100, 7);
    auto cpm_text_width = M5.Lcd.drawString(" CPM", 20 + cpm_width, 105, 4); // Prints after cpm value
    int i = 0;
    while (320 - (20 + cpm_width + cpm_text_width + i) > 0) {
      // Fill rest of the line with empty spaces
      i += M5.Lcd.drawString(" ", 20 + cpm_width + cpm_text_width + i, 105, 4);
    }

    // Display uSv/h
    auto ush_width = printFloatFont(gm_sensor->get_data().uSv, 3, 20, 140, 4);
    auto ush_text_width = M5.Lcd.drawString(" uSv/h", 20 + ush_width, 140, 4); // Prints after ush value
    i = 0;
    while (320 - (20 + ush_width + ush_text_width + i) > 0) {
      // Fill rest of the line with empty spaces
      i += M5.Lcd.drawString(" ", 20 + ush_width + ush_text_width + i, 140, 4);
    }
  }

  // Display GPS data always, change colour if not fresh

  if (gps->get_data().location_valid) {
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  }
  else {
    M5.Lcd.setTextColor(LCD_COLOR_OLD, LCD_COLOR_BACKGROUND);
  }

  if (gps->is_fresh() || force) {
    M5.Lcd.setCursor(0, 150);
    gps->get_data().satsInView < 2 ? (M5.Lcd.setTextColor(TFT_RED, TFT_BLACK))
                                   : M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Lcd.print("Satellites :");
    M5.Lcd.print(gps->get_data().satsInView, 5);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.println();
    M5.Lcd.print("Latitude   :");
    M5.Lcd.print(gps->get_data().latitude, 6);
    M5.Lcd.println();
    M5.Lcd.print("Longitude  :");
    M5.Lcd.print(gps->get_data().longitude, 6);
    M5.Lcd.println();
    M5.Lcd.print("Altitude   :");
    M5.Lcd.print(gps->get_data().altitudeMSL, 2);
    M5.Lcd.println();
  }
}

void DriveModeScreen::leave_screen(Controller& controller) {
  // TODO: close logging to file
}
