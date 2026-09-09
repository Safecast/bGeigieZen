#include "gps_settings.h"
#include "config_mode.h"
#include "identifiers.h"
#include "user_config.h"
#include "utils/sd_wrapper.h"
#include "workers/gps_connector.h"
#include "workers/local_storage.h"
#include "workers/zen_button.h"

GpsSettingsScreen GpsSettingsScreen_i;

static const GpsSettingsScreen::MenuItem GPS_MENU[GpsSettingsScreen::e_gps_MENU_MAX] = {
    {.title="Set Home GPS", .tooltip="Set current GPS location as home for Real Time mode", .enabled=true, .screen=nullptr},
    {.title="Fixed Range", .tooltip="Adjust the Real Time home range distance", .enabled=true, .screen=nullptr},
    {.title="Back", .tooltip="Return to Settings", .enabled=true, .screen=&ConfigModeScreen_i},
};

GpsSettingsScreen::GpsSettingsScreen() : BaseScreenWithMenu("GPS", true) {
  _current_page = e_gps_MENU_MAX;  // sentinel: "show menu, no action page"
}

void GpsSettingsScreen::enter_screen(Controller& controller) {
  if (_current_page == e_gps_MENU_MAX) {
    _menu_index = 0;
    open_menu(true);
  }
  force_next_render();
}

void GpsSettingsScreen::leave_screen(Controller& controller) {
  _current_page = e_gps_MENU_MAX;
}

BaseScreen* GpsSettingsScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  if (menu_open()) {
    return handle_menu_input(controller, workers, GPS_MENU, e_gps_MENU_MAX);
  }

  auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);

  if (_current_page == e_gps_page_fixed_range) {
    auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
    if (button1->is_fresh() && button1->get_data().shortPress) {
      float step = button1->get_data().longPress ? 0.5f : 0.1f;
      float new_value = settings->get_fixed_range() - step;
      if (new_value < 0) new_value = 0;
      settings->set_fixed_range(new_value, false);
      if (SDInterface::i().ready()) {
        SDInterface::i().write_safezen_file_from_settings(*settings, false);
      }
      force_next_render();
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      float step = button2->get_data().longPress ? 0.5f : 0.1f;
      float new_value = settings->get_fixed_range() + step;
      if (new_value > 5.0f) new_value = 5.0f;
      settings->set_fixed_range(new_value, false);
      if (SDInterface::i().ready()) {
        SDInterface::i().write_safezen_file_from_settings(*settings, false);
      }
      force_next_render();
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      open_menu(true);
      M5.Lcd.clear(LCD_COLOR_BACKGROUND);
      force_next_render();
    }
  }

  if (_current_page == e_gps_page_set_home) {
    if (button2->is_fresh() && button2->get_data().shortPress) {
      auto* gps = workers.worker<GpsConnector>(k_worker_gps_connector);
      auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);

      if (gps && gps->active() && gps->get_data().location_valid) {
        double new_lat = gps->get_data().latitude;
        double new_lon = gps->get_data().longitude;

        settings->set_fixed_latitude(new_lat, false);
        settings->set_fixed_longitude(new_lon, false);
        settings->set_last_latitude(new_lat, false);
        settings->set_last_longitude(new_lon, false);

        if (SDInterface::i().ready()) {
          SDInterface::i().write_safezen_file_from_settings(*settings, false);
        }

        set_status_message(F(" HOME GPS LOCATION SET! "));
        force_next_render();
      } else {
        set_status_message(F(" NO VALID GPS FIX! "));
      }
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      open_menu(true);
      M5.Lcd.clear(LCD_COLOR_BACKGROUND);
      force_next_render();
    }
  }
  return nullptr;
}

void GpsSettingsScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!force) {
    return;
  }
  if (menu_open()) {
    render_menu(GPS_MENU, e_gps_MENU_MAX, true, 1);
    return;
  }
  clear_screen_content();
  if (_current_page == e_gps_page_set_home) {
    render_set_home_gps_page(workers, handlers);
  } else if (_current_page == e_gps_page_fixed_range) {
    render_fixed_range_page(workers, handlers);
  }
}

void GpsSettingsScreen::render_fixed_range_page(const worker_map_t& workers, const handler_map_t& handlers) {
  auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);

  drawButton1("-0.1km\nHold:-0.5");
  drawButton2("+0.1km\nHold:+0.5");
  drawButton3("Back");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 50, &fonts::Font2);
  M5.Lcd.printf("Fixed Range\n\n");
  M5.Lcd.printf("Current: %0.1f km\n\n", settings->get_fixed_range());
  M5.Lcd.printf("Distance from home location\n");
  M5.Lcd.printf("considered 'in range' for\n");
  M5.Lcd.printf("Real Time mode.\n\n");
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Range: 0.0-5.0 km\n");
}

void GpsSettingsScreen::render_set_home_gps_page(const worker_map_t& workers, const handler_map_t& handlers) {
  auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
  auto* gps = workers.worker<GpsConnector>(k_worker_gps_connector);

  drawButton1("");
  drawButton2("Set Home");
  drawButton3("Back");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 50, &fonts::Font2);
  M5.Lcd.printf("Set Home GPS Location\n\n");

  M5.Lcd.printf("Current Home (Real Time):\n");
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Lat: %.6f\n", settings->get_fixed_latitude());
  M5.Lcd.printf("Lon: %.6f\n", settings->get_fixed_longitude());

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Current GPS Position:\n");

  if (gps && gps->active() && gps->get_data().location_valid) {
    M5.Lcd.setTextColor(LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("Lat: %.6f\n", gps->get_data().latitude);
    M5.Lcd.printf("Lon: %.6f\n", gps->get_data().longitude);
  } else if (gps && gps->active()) {
    M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("Waiting for GPS fix...\n");
    M5.Lcd.printf("Sats in view: %d\n", gps ? gps->get_data().satsInView : 0);
  } else {
    M5.Lcd.setTextColor(LCD_COLOR_ERROR, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("GPS not available\n");
  }

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("\nPress B to set current GPS\n");
  M5.Lcd.printf("position as home location.\n");
}
