#include "base_screen.h"
#include "identifiers.h"
#include "user_config.h"
#include "utils/wifi_connection.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"

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

void BaseScreen::drawButton(uint16_t x, const char* text, ButtonState state) const {
  if (strlen(text) > BUTTON_TEXT_MAX_LENGTH) {
    // Dont render long button text :(
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
  M5.Lcd.drawString(blob, (x + 45) - static_cast<uint16_t>(strlen(blob) * 3), 10);
  M5.Lcd.flush();
}

void BaseScreen::drawButton1(const char* text, ButtonState state) const {
#ifdef M5_CORE2
  drawButton(10, text, state);
#elif M5_BASIC
  drawButton(20, text, state);
#endif
}

void BaseScreen::drawButton2(const char* text, ButtonState state) const {
#ifdef M5_CORE2
  drawButton(115, text, state);
#elif M5_BASIC
  drawButton(115, text, state);
#endif
}

void BaseScreen::drawButton3(const char* text, ButtonState state) const {
#ifdef M5_CORE2
  drawButton(220, text, state);
#elif M5_BASIC
  drawButton(210, text, state);
#endif
}

int BaseScreen::printFloatFont(float val, int prec, int x, int y, int font) const {
  char sz[32] = "";
  char format[32] = "";
  sprintf(format, "%%.%df", prec);
  sprintf(sz, format, val);
  return M5.Lcd.drawString((sz), x, y, font);
}

// Prints int with fonts
int BaseScreen::printIntFont(unsigned long val, int x, int y, int font) const {
  char sz[32] = "";
  sprintf(sz, "%ld", val);
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
  if (required_tube && millis() > 2000 && !workers.worker<GeigerCounter>(k_worker_gm_sensor)->active()) {
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

const __FlashStringHelper* BaseScreen::get_status_message(const worker_map_t& workers, const handler_map_t& handlers) const {
  if (_message && _status_message_time && _status_message_time + STATUS_MESSAGE_DURATION > millis()) {
    return _message;
  }
  return nullptr;
}

void BaseScreen::set_status_message(const __FlashStringHelper* message) {
  _message = message;
  _status_message_time = millis();
}
