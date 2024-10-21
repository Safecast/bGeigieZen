#include "handlers/sd_logger.h"
#include "identifiers.h"
#include "menu_window.h"
#include "survey_mode.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/zen_button.h"

SurveyModeScreen SurveyModeScreen_i;

SurveyModeScreen::SurveyModeScreen() : BaseScreen("Survey", true), _logging_available(false), _currently_logging(false) {
  required_tube = true;
  required_gps = true;
  required_sd = true;
}

BaseScreen* SurveyModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto log_button = workers.worker<ZenButton>(k_worker_button_1);
  if (_logging_available && log_button->is_fresh() && log_button->get_data().shortPress) {
    controller.set_handler_active(k_handler_survey_logger, !_currently_logging);
  }

  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return &MenuWindow_i;
  }
  return nullptr;
}

void SurveyModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  const auto& controller_data = workers.worker<Controller>(k_worker_device_state)->get_data();
  _logging_available = controller_data.local_available && SDInterface::i().status() == SDInterface::e_sd_config_status_ok;
  bool currently_logging = handlers.handler<SdLogger>(k_handler_survey_logger)->active();
  if (_currently_logging && !currently_logging) {
    set_status_message(F(" COMPLETED LOGGING SURVEY "));
  } else if (!_currently_logging && currently_logging) {
    set_status_message(F(" STARTED LOGGING, stay safe! "));
  }
  _currently_logging = currently_logging;

  if (_logging_available && _currently_logging) {
    // Is logging
    drawButton1("Stop survey");
  }
  else {
    drawButton1("Start survey", _logging_available ? e_button_default : e_button_disabled);
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
      uint16_t cpm_width = printIntFont(gm_sensor->get_data().cp5s, 0, 100, &fonts::Font7);
      uint16_t ush_width = printFloatFont(gm_sensor->get_data().uSvh_5sec, 4, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (CPM uSv/h)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(cpm_width, 52, 320 - cpm_width, 27, LCD_COLOR_BACKGROUND); // Prints blanks after cpm value, above CPM text
      cpm_width += M5.Lcd.drawString(" CP5S", cpm_width, 105, &fonts::Font4); // Prints after cpm value
      M5.Lcd.fillRect(cpm_width, 74, 320 - cpm_width, 26, LCD_COLOR_BACKGROUND); // Prints blanks after CPM text
      M5.Lcd.drawString(" uSv/h   ", 0 + ush_width, 140, &fonts::Font4); // Prints after ush value
    } else {
      M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      uint16_t ush_width = printFloatFont(gm_sensor->get_data().uSvh_5sec, 3, 0, 100, &fonts::Font7);
      uint16_t cpm_width = printIntFont(gm_sensor->get_data().cp5s, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (CPM uSv/h)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(ush_width, 52, 320 - ush_width, 27, LCD_COLOR_BACKGROUND); // Prints blanks after cpm value, above CPM text
      ush_width += M5.Lcd.drawString(" uSv/h", ush_width, 105, &fonts::Font4); // Prints after cpm value
      M5.Lcd.fillRect(ush_width, 74, 320 - ush_width, 26, LCD_COLOR_BACKGROUND); // Prints blanks after CPM text
      M5.Lcd.drawString(" CP5S   ", 0 + cpm_width, 140, &fonts::Font4); // Prints after ush value
    }
  }


  if (gps->is_fresh() || force) {
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

void SurveyModeScreen::enter_screen(Controller& controller) {
  if (!controller.get_settings().get_manual_logging()) {
    // Automatically start logging
    controller.set_handler_active(k_handler_survey_logger, true);
  }
}

void SurveyModeScreen::leave_screen(Controller& controller) {
  // close logging to file
  controller.set_handler_active(k_handler_survey_logger, false);
}
