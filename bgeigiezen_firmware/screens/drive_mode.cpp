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

DriveModeScreen::DriveModeScreen() : BaseScreen("Drive", true), _logging_available(false), _currently_logging(false) {
}

BaseScreen* DriveModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto log_button = workers.worker<ZenButton>(k_worker_button_1);
  if (_logging_available && log_button->is_fresh() && log_button->get_data().shortPress) {
    controller.set_handler_active(k_handler_drive_logger, !_currently_logging);
  }

  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return MenuWindow::i();
  }

  // TODO: handle something like this
//  if (controller.is_fresh() && controller.get_data().sd_card_status == SDInterface::SdStatus::e_sd_config_status_config_different_id) {
//    return SdMessageScreen::i();
//  }

  return nullptr;
}

void DriveModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  const auto& controller_data = workers.worker<Controller>(k_worker_device_state)->get_data();
  _logging_available = controller_data.local_available && controller_data.sd_card_status == SDInterface::e_sd_config_status_ok;
  _currently_logging = handlers.handler<SdLogger>(k_handler_drive_logger)->active();
  /// Menu
  if (_logging_available && _currently_logging) {
    // Is logging
    drawButton1("Stop log");
  }
  else {
    drawButton1("Start log", _logging_available ? e_button_default : e_button_disabled);
  }
  drawButton2("");
  drawButton3("Menu");

  /// Display drive data
  const auto& gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);
  const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);


  if (gm_sensor->is_fresh() || force) {
    M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_OLD, LCD_COLOR_BACKGROUND);

    // Clean CPM area
    M5.Lcd.fillRect(0, 52, 320, 90, LCD_COLOR_BACKGROUND);

    // Display CPM
    auto cpm_width = printIntFont(gm_sensor->get_data().cpm_comp, 20, 100, 7);
    M5.Lcd.drawString(" CPM", 20 + cpm_width, 105, 4); // Prints after cpm value

    // Display uSv/h
    auto ush_width = printFloatFont(gm_sensor->get_data().uSv, 3, 20, 140, 4);
    M5.Lcd.drawString(" uSv/h", 20 + ush_width, 140, 4); // Prints after ush value
  }

  // Display GPS data always, change colour if not fresh

  if (gps->is_fresh() || force) {
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_OLD, LCD_COLOR_BACKGROUND);

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
  // close logging to file
  controller.set_handler_active(k_handler_drive_logger, false);
}
