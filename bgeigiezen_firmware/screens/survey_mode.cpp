#include "survey_mode.h"
#include "handlers/sd_logger.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/battery_indicator.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/log_aggregator.h"
#include "workers/zen_button.h"
#include "controller.h"

SurveyModeScreen SurveyModeScreen_i;

SurveyModeScreen::SurveyModeScreen() : BaseScreen("Survey Mode", true), _logging_available(false), _currently_logging(false), _previous_gps_model(DYNMODEL_PORT), _gps_model_set(false) {
  required_tube = true;
  required_gps = true;  // GPS is used in Survey mode
  required_wifi = false;  // WiFi is optional for Survey mode
  required_sd = true;
}

BaseScreen* SurveyModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto log_button = workers.worker<ZenButton>(k_worker_button_1);
  if (_logging_available && log_button->is_fresh() && log_button->get_data().shortPress) {
    controller.set_handler_active(k_handler_survey_logger, !_currently_logging);
  }

  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    // Restore previous GPS dynamic platform model before returning to menu
    auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
    if (gps) {
      gps->setDynamicModel(_previous_gps_model);
      set_status_message(F(" LEAVING SURVEY MODE "));
    }
    return &MenuWindow_i;
  }
  return nullptr;
}

void SurveyModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  const auto& controller_data = workers.worker<Controller>(k_worker_device_state)->get_data();
  _logging_available = controller_data.local_available && SDInterface::i().status() == SDInterface::e_sd_config_status_ok;

  // Set GPS to PEDESTRIAN mode on first render
  static bool first_render = true;
  if (first_render) {
    auto gps = workers.worker<GpsConnector>(k_worker_gps_connector);
    if (gps) {
      // Save current dynamic model to restore when leaving
      _previous_gps_model = gps->getDynamicModel();
      
      // Set to PEDESTRIAN mode and save to flash memory
      if (gps->setDynamicModel(DYNMODEL_PEDESTRIAN, true)) {
        set_status_message(F(" SURVEY MODE - GPS SET TO PEDESTRIAN (SAVED TO FLASH) "));
        _gps_model_set = true; // Mark that we've changed the GPS model
      }
    }
    first_render = false;
    force = true; // Force a full render on first display
  }

  // Check if we're currently logging
  bool currently_logging = handlers.handler<SdLogger>(k_handler_survey_logger)->active();
  if (_currently_logging && !currently_logging) {
    set_status_message(F(" COMPLETED LOGGING SURVEY "));
  } else if (!_currently_logging && currently_logging) {
    set_status_message(F(" STARTED LOGGING SURVEY "));
  }

  if (currently_logging != _currently_logging) {
    _currently_logging = currently_logging;
    force_next_render();
  }

  // Menu buttons
  if (_logging_available && _currently_logging) {
    // Is logging
    drawButton1("Stop survey");
  }
  else {
    drawButton1("Start survey", _logging_available ? e_button_default : e_button_disabled);
  }
  drawButton2("");
  drawButton3("Menu");

  // Get references to sensors
  const auto& gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);
  const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);

  // Clear screen content on force render
  if (force) {
    clear_screen_content();
  }

  // Always update the radiation data when fresh
  if (gm_sensor->is_fresh() || force) {
    // Display values based on user settings
    const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);
    
    // Use the 5-second average (cp5s) instead of single-second CPS to avoid showing 0
    // The cp5s value is the sum of counts over the last 5 seconds
    // We'll use the 5-second uSv/h value that's already calculated in the sensor
    float cps_usvh = gm_sensor->get_data().uSvh_5sec; // Already calculated 5-second average in uSv/h
    
    if (settings->get_cpm_usvh()) {
      // Display CP5S (5-second average) big, uSv/h small for Survey Mode
      M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      uint16_t cps_width = printIntFont(gm_sensor->get_data().cp5s, 0, 100, &fonts::Font7);
      uint16_t ush_width = printFloatFont(cps_usvh, 4, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (CP5S uSv/h)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(cps_width, 52, 320 - cps_width, 27, LCD_COLOR_BACKGROUND); // Prints blanks after cps value
      cps_width += M5.Lcd.drawString(" CP5S", cps_width, 105, &fonts::Font4); // Prints after cps value
      M5.Lcd.fillRect(cps_width, 74, 320 - cps_width, 26, LCD_COLOR_BACKGROUND); // Prints blanks after CP5S text
      M5.Lcd.drawString(" uSv/h   ", 0 + ush_width, 140, &fonts::Font4); // Prints after ush value
    } else {
      // Display uSv/h big, CP5S small
      M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      uint16_t ush_width = printFloatFont(cps_usvh, 3, 0, 100, &fonts::Font7);
      uint16_t cps_width = printIntFont(gm_sensor->get_data().cp5s, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (uSv/h CP5S)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(ush_width, 52, 320 - ush_width, 27, LCD_COLOR_BACKGROUND); // Prints blanks after uSv/h value
      ush_width += M5.Lcd.drawString(" uSv/h", ush_width, 105, &fonts::Font4); // Prints after uSv/h value
      M5.Lcd.fillRect(ush_width, 74, 320 - ush_width, 26, LCD_COLOR_BACKGROUND); // Prints blanks after uSv/h text
      M5.Lcd.drawString(" CP5S   ", 0 + cps_width, 140, &fonts::Font4); // Prints after cp5s value
    }
  }

  // Always update GPS data when fresh
  if (gps->is_fresh() || force) {
    // Print survey data
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    const uint16_t lat_width = M5.Lcd.drawString("Latitude  :", 0, 157, &fonts::Font0);
    const uint16_t lon_width = M5.Lcd.drawString("Longitude :", 0, 165, &fonts::Font0);
    const uint16_t sat_width = M5.Lcd.drawString("Satellites:", 0, 173, &fonts::Font0);

    // Print GPS values
    M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawFloat(gps->get_data().latitude, 6, lat_width + 5, 157, &fonts::Font0);
    M5.Lcd.drawFloat(gps->get_data().longitude, 6, lon_width + 5, 165, &fonts::Font0);
    
    // Make sure satellites are displayed correctly
    uint8_t satellites = gps->get_data().numSV;
    if (satellites == 0 && gps->is_fresh()) {
      // If numSV is 0 but GPS is fresh, try to get satellites from satsInView
      satellites = gps->get_data().satsInView;
    }
    M5.Lcd.drawNumber(satellites, sat_width + 5, 173, &fonts::Font0);
  }
}

void SurveyModeScreen::enter_screen(Controller& controller) {
  BaseScreen::enter_screen(controller);
  
  if (!controller.get_settings().get_manual_logging()) {
    // Automatically start logging
    controller.set_handler_active(k_handler_survey_logger, true);
  }
  
  // We'll set the GPS to PEDESTRIAN mode in the first render call
  // when we have access to the worker map and save it to flash memory
  set_status_message(F(" SURVEY MODE - GPS SET TO PEDESTRIAN (SAVED TO FLASH) "));
  force_next_render(); // Force render to apply GPS settings
}

void SurveyModeScreen::leave_screen(Controller& controller) {
  BaseScreen::leave_screen(controller);
  
  // close logging to file
  controller.set_handler_active(k_handler_survey_logger, false);
  
  // We can't access the GPS connector directly from here
  // The GPS model will be restored in the handle_input method
  // when returning to the menu
}
