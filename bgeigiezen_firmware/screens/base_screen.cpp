#include "base_screen.h"
#include "identifiers.h"
#include "utils/wifi_connection.h"
#include "utils/error_beep.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/local_storage.h"
#include "workers/sound_manager.h"
#include "drive_mode.h"
#include "survey_mode.h"
#include "flight_mode.h"
#include "fixed_mode.h"
#include "satellite_view.h"

#include <WiFi.h>

#define BUTTON_TEXT_MAX_LENGTH 14

BaseScreen::BaseScreen(const char* title, bool status_bar)
    : _status_bar(status_bar),
      required_gps(false),
      required_sd(false),
      required_tube(false),
      required_wifi(false),
      required_ble(false) {
  if (strlen(title) < 20) {
    strcpy(_title, title);
  }
}


void BaseScreen::drawButton(uint16_t x, const char* text, ButtonState state) {
  if (strlen(text) > BUTTON_TEXT_MAX_LENGTH) {
    // Don't render long button text :(
    return;
  }

  int border_color = LCD_COLOR_BACKGROUND;
  int text_color = LCD_COLOR_STALE_INCOMPLETE;

  if (strlen(text)) {
    switch (state) {
      case e_button_default:
        border_color = LCD_COLOR_DEFAULT;
        break;
      case e_button_active:
        border_color = LCD_COLOR_STALE_INCOMPLETE;
        break;
      case e_button_disabled:
        border_color = LCD_COLOR_INACTIVE;
        text_color = LCD_COLOR_INACTIVE;
        break;
    }
  }

  M5.Lcd.drawRoundRect(x, -5, 90, 20, 4, border_color);
  M5.Lcd.setTextColor(text_color, LCD_COLOR_BACKGROUND);

  // Center the button text, making sure around the text is whitespace to clear previous texts
  char blob[BUTTON_TEXT_MAX_LENGTH + 1] = "              ";
  for (size_t i = 0; i < strlen(text); ++i) {
    blob[i + ((BUTTON_TEXT_MAX_LENGTH - strlen(text)) / 2)] = text[i];
  }
  if (strlen(text) % 2 != BUTTON_TEXT_MAX_LENGTH % 2) {
    blob[BUTTON_TEXT_MAX_LENGTH - 1] = '\0';
  }
  M5.Lcd.drawString(blob, (x + 45) - static_cast<uint16_t>(strlen(blob) * 3), 10, &fonts::Font0);
  M5.Lcd.flush();
}

void BaseScreen::drawButton1(const char* text, ButtonState state) const {
  drawButton(10, text, state);
}

void BaseScreen::drawButton2(const char* text, ButtonState state) const {
  drawButton(115, text, state);
}

void BaseScreen::drawButton3(const char* text, ButtonState state) const {
  drawButton(220, text, state);
}

size_t BaseScreen::printFloatFont(float val, int prec, int x, int y, const lgfx::IFont* font) {
  char sz[32] = "";
  char format[32] = "";
  sprintf(format, "%%.%df", prec);
  sprintf(sz, format, val);
  return M5.Lcd.drawString((sz), x, y, font);
}

// Prints int with fonts
size_t BaseScreen::printIntFont(unsigned long val, int x, int y, const lgfx::IFont* font) {
  char sz[32] = "";
  sprintf(sz, "%lu", val);  // Use %lu for unsigned long instead of %ld
  return M5.Lcd.drawString((sz), x, y, font);
}

void BaseScreen::enter_screen(Controller& controller) {
}

void BaseScreen::leave_screen(Controller& controller) {
}

const char* BaseScreen::get_title() const {
  return _title;
}

bool BaseScreen::has_status_bar() const {
  return _status_bar;
}

void BaseScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
}

void BaseScreen::do_render(const worker_map_t& workers, const handler_map_t& handlers) {
  render(workers, handlers, _force_render);
  _force_render = false;
}

void BaseScreen::force_next_render() {
  _force_render = true;
}

const __FlashStringHelper* BaseScreen::get_error_message(const worker_map_t& workers, const handler_map_t& handlers) const {
  if (required_tube && millis() > 6000 && !workers.worker<GeigerCounter>(k_worker_gm_sensor)->active()) {
    return STATUS_ERROR_GEIGER;
  }
  if (required_gps && !workers.worker<GpsConnector>(k_worker_gps_connector)->active()) {
    return STATUS_ERROR_GPS;
  }
  if (required_sd && !SDInterface::i().can_write_logs()) {
    return STATUS_ERROR_SD;
  }
  if (required_wifi && WiFiWrapper_i.status() == WL_NO_SSID_AVAIL) {
    return STATUS_ERROR_WIFI_NO_SSID_AVAIL;
  }
  if (required_wifi && WiFiWrapper_i.status() == WL_CONNECT_FAILED) {
    return STATUS_ERROR_WIFI_CONNECT_FAILED;
  }
  if (required_wifi && WiFiWrapper_i.status() == WL_CONNECTION_LOST) {
    return STATUS_ERROR_WIFI_CONNECTION_LOST;
  }

  return nullptr;
}

const __FlashStringHelper* BaseScreen::get_status_message(const worker_map_t& workers, const handler_map_t& handlers) {
  // First check for persistent CPM alert message
  auto* gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);
  if (gm_sensor && gm_sensor->get_data().alert) {
    // CPM is above threshold - show persistent alert message
    return F(" ⚠ CPM ALERT ACTIVE ⚠ ");
  }
  
  // Check if temporary message exists and hasn't timed out
  if (_message) {
    unsigned long current_millis = millis();
    long time_elapsed = current_millis - _status_message_time;
    if (_status_message_time && (time_elapsed < STATUS_MESSAGE_DURATION)) {
      return _message;
    } else {
      _message = nullptr;
      _status_message_time = 0;
    }
  }
  return nullptr;
}

void BaseScreen::set_status_message(const __FlashStringHelper* message) {
  _message = message;
  _status_message_time = millis();
}

void BaseScreen::clear_screen_content() {
  M5.Lcd.fillRect(0, 20, 320, 180, LCD_COLOR_BACKGROUND);
}

BaseScreen* BaseScreenWithMenu::handle_menu_input(Controller& controller, const worker_map_t& workers, const MenuItem items[], int menu_max) {
  auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);

  // Button 1 is move index down
  if (button1->is_fresh() && button1->get_data().shortPress) {
    ++_menu_index;
    _menu_index %= menu_max;
    force_next_render();
  }

  // Button 2 move index up
  if (button2->is_fresh() && button2->get_data().shortPress) {
    _menu_index = _menu_index + menu_max - 1;
    _menu_index %= menu_max;
    force_next_render();
  }

  // Button 3 change view
  if (button3->is_fresh() && button3->get_data().shortPress && items[_menu_index].enabled) {
    // Save the selected mode to LocalStorage
    auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
    if (items[_menu_index].screen == &SurveyModeScreen_i) {
      settings->set_last_mode(LocalStorage::e_operational_mode_survey, true);
    } else if (items[_menu_index].screen == &DriveModeScreen_i) {
      settings->set_last_mode(LocalStorage::e_operational_mode_drive, true);
    } else if (items[_menu_index].screen == &FlightModeScreen_i) {
      settings->set_last_mode(LocalStorage::e_operational_mode_flight, true);
    } else if (items[_menu_index].screen == &FixedModeScreen_i) {
      settings->set_last_mode(LocalStorage::e_operational_mode_fixed, true);
    } else if (items[_menu_index].screen == &SatelliteViewScreen_i) {
      settings->set_last_mode(LocalStorage::e_operational_mode_satellite, true);
    }

    if (items[_menu_index].screen) {
      // Swap screens
      return items[_menu_index].screen;
    } else {
      // Swap pages internally
      _menu_open = false;
      M5.Lcd.clear();
      if (_menu_index != _current_page) {
        leave_screen(controller);
        _current_page = _menu_index;
        enter_screen(controller);
      }
      force_next_render();
    }

  }

  return nullptr;
}

void BaseScreenWithMenu::render_menu(const MenuItem items[], int menu_max, bool outline, int outline_button) {
  // Draw buttons
  drawButton1("Down");
  drawButton2("Up");
  drawButton3("Enter", items[_menu_index].enabled ? e_button_active : e_button_disabled);

  if (outline) {
    // Draw menu overlay border
    M5.Lcd.drawRoundRect(10, 20, 300, 170, 4, items[_menu_index].enabled ? LCD_COLOR_STALE_INCOMPLETE : LCD_COLOR_INACTIVE);
    if (outline_button == 3) {
      // Visually connect button to overlay
      M5.Lcd.fillRect(220, 12, 90, 12, LCD_COLOR_BACKGROUND);
      M5.Lcd.drawLine(220, 12, 220, 20, items[_menu_index].enabled ? LCD_COLOR_STALE_INCOMPLETE : LCD_COLOR_INACTIVE);
      M5.Lcd.drawLine(309, 12, 309, 23, items[_menu_index].enabled ? LCD_COLOR_STALE_INCOMPLETE : LCD_COLOR_INACTIVE);
    }
  }

  // Clear menu area to prevent overlapping text
  M5.Lcd.fillRect(16, 26, 144, 158, LCD_COLOR_BACKGROUND);
  
  // Draw tooltip block
  M5.Lcd.fillRoundRect(161, 26, 142, 158, 4, LCD_COLOR_BACKGROUND);
  // Draw separate line between menu and tooltip
  M5.Lcd.drawLine(160, 33, 160, 177, LCD_COLOR_STALE_INCOMPLETE);

  // Calculate visible range - show at most 9 items to fit on screen
  int startIdx = max(0, _menu_index - 4);
  int endIdx = min(menu_max, startIdx + 9);
  
  // Adjust startIdx if we have fewer than 9 items at the end
  if (endIdx - startIdx < 9 && startIdx > 0) {
    startIdx = max(0, endIdx - 9);
  }
  
  for (int i = startIdx; i < endIdx; ++i) {
    int yPos = 48 + ((i - startIdx) * 16); // Adjust y position based on visible range
    M5.Lcd.setTextColor(items[i].enabled ? (i == _menu_index ? LCD_COLOR_STALE_INCOMPLETE : LCD_COLOR_DEFAULT) : LCD_COLOR_INACTIVE, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawLine(16, yPos, 159, yPos, (i == _menu_index ? (items[i].enabled ? LCD_COLOR_STALE_INCOMPLETE : LCD_COLOR_INACTIVE) : LCD_COLOR_BACKGROUND));
    M5.Lcd.setCursor(16, yPos + 8, &fonts::Font2);
    if (i == _menu_index) {
      M5.Lcd.print("> ");
    }
    else {
      M5.Lcd.print("  ");
    }
    M5.Lcd.print(items[i].title);
    M5.Lcd.print("  ");
  }
  
  // Show scroll indicators if needed
  if (startIdx > 0) {
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawString("▲", 140, 40, &fonts::Font2);
  }
  if (endIdx < menu_max) {
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawString("▼", 140, 184, &fonts::Font2);
  }

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(170, 46);
  M5.Lcd.println(items[_menu_index].title);
  size_t tooltip_length = strlen(items[_menu_index].tooltip);
  uint8_t line = 0;
  uint8_t current_line_length = 0;
  M5.Lcd.setCursor(170, 76);
  for (size_t i = 0; i < tooltip_length; ++i) {
    if (current_line_length == 0 && items[_menu_index].tooltip[i] == ' ') {
      continue;
    }
    current_line_length += M5.Lcd.drawChar(items[_menu_index].tooltip[i], 170 + current_line_length, 76 + (line * 16), 2);
    if (current_line_length > 124) {
      line += 1;
      current_line_length = 0;
    }
  }
  if (!items[_menu_index].enabled) {
    M5.Lcd.setTextColor(LCD_COLOR_ERROR, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawString("Not yet available", 170, 110 + (line * 16), &fonts::Font2);
  }

  M5.Lcd.setCursor(0, 0, &fonts::Font0);

}

bool BaseScreenWithMenu::menu_open() const {
  return _menu_open;
}

void BaseScreenWithMenu::open_menu(bool open) {
  _menu_open = open;
}
