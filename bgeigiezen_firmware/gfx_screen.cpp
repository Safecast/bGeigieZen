#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

#include "controller.h"
#include "debugger.h"
#include "gfx_screen.h"
#include "identifiers.h"
#include "screens/boot_screen.h"
#include "screens/default_entry_screen.h"
#include "utils/wifi_connection.h"
#include "workers/battery_indicator.h"
#include "workers/gm_sensor.h"
#include "workers/rtc_connector.h"
#include "workers/zen_button.h"

#define SCREENSAVER_ENABLED_DEFAULT false
#define SCREENSAVER_TEXT VERSION_STRING
#define SCREENSAVER_TEXT_LENGTH (strlen(SCREENSAVER_TEXT) * 6)
static constexpr uint8_t LEVEL_BRIGHT = 80;  // max brightness = 100
static constexpr uint8_t LEVEL_DIMMED = 30;
static constexpr uint8_t LEVEL_BLANKED = 0;
static constexpr uint32_t DELAY_DIMMING_DEFAULT = 59 * 1000;  // 59 seconds (before dimming screen)
static constexpr uint32_t DELAY_BLANKING_DEFAULT = 10 * 60 * 1000;  // 10 minutes (before blanking/saving screen)


GFXScreen::GFXScreen(LocalStorage& settings, Controller& controller)
    : Supervisor(),
      _saver(&M5.Lcd),
      _controller(controller),
      _settings(settings),
      _last_render(0),
      _last_interaction(0),
      _saver_enabled(SCREENSAVER_ENABLED_DEFAULT),
      _saver_x(140),
      _saver_y(115),
      _saver_x_direction(1),
      _saver_y_direction(1),
      _screen_status(e_screen_status_on),
      _screen(nullptr),
      _menu(&MenuWindow_i) {
}

void GFXScreen::initialize() {
#ifdef M5_CORE2
  // Change touch screen buttons slightly to overlap with button indicators
  M5.BtnA.set(10, 230, 90, 50);
  M5.BtnB.set(115, 230, 90, 50);
  M5.BtnC.set(220, 230, 90, 50);
#endif

  _saver.createSprite(4 + SCREENSAVER_TEXT_LENGTH, 12);
  _saver.fillScreen(LCD_COLOR_BACKGROUND);
  _saver.drawString(SCREENSAVER_TEXT, 2, 2, 1);
  _screen = &BootScreen_i;
  _screen->enter_screen(_controller);
  set_screen_status(e_screen_status_on);
  clear();
}

void GFXScreen::set_screen_status(ScreenStatus status) {
  _screen_status = status;
  switch (_screen_status) {
    case e_screen_status_on:
      setBrightness(LEVEL_BRIGHT);
      break;
    case e_screen_status_dim:
      setBrightness(LEVEL_DIMMED);
      break;
    case e_screen_status_off:
      M5.Lcd.clear();
      setBrightness(_saver_enabled ? LEVEL_DIMMED : LEVEL_BLANKED);
      break;
  }
}

//setup brightness by Rob Oudendijk 2023-03-13
void GFXScreen::setBrightness(uint8_t lvl, bool overdrive) {
#ifdef M5_CORE2

  if (lvl == LEVEL_BLANKED) {
    // Turn off screen
    M5.Axp.SetDCDC3(false);
  } else {
    // Make sure screen is turned on
    M5.Axp.SetDCDC3(true);
    // Set brightness
    M5.Axp.ScreenBreath(lvl);
  }

#elif M5_BASIC
  // M5Stack Basic uses LCD Brightness (0: Off - 255:Full)
  int v = lvl == LEVEL_DIMMED ? 1 : lvl * 25;
  M5.Lcd.setBrightness(v);
#endif
}

void GFXScreen::clear() {
  // Clear display

  M5.Lcd.startWrite();
  M5.Lcd.clear();
  M5.Lcd.setTextDatum(BL_DATUM);  // By default, text x,y is bottom left corner
  M5.Lcd.setTextFont(1);
  if (_screen) {
    _screen->force_next_render();
  }
  M5.Lcd.endWrite();

}

void GFXScreen::handle_report(const worker_map_t& workers, const handler_map_t& handlers) {
  if (_screen) {

    // Keep track of screen interaction / wake up
    const auto button1 = workers.worker<ZenButton>(k_worker_button_1);
    const auto button2 = workers.worker<ZenButton>(k_worker_button_2);
    const auto button3 = workers.worker<ZenButton>(k_worker_button_3);
    const auto screen_touch = workers.worker<ZenButton>(k_worker_screen_touch);

    if (button1->is_fresh() || button2->is_fresh() || button3->is_fresh() || screen_touch->is_fresh()) {
      _last_interaction = millis();
    }

    // Will be set false when returning from wake up
    bool handle_input = true;

    switch (_screen_status) {
      case e_screen_status_on:
        if (millis() - _last_interaction > DELAY_DIMMING_DEFAULT) {
          set_screen_status(e_screen_status_dim);
        }
        break;
      case e_screen_status_dim:
        if (millis() - _last_interaction > DELAY_BLANKING_DEFAULT) {
          clear();
          set_screen_status(e_screen_status_off);
        }
        if (millis() - _last_interaction < DELAY_DIMMING_DEFAULT) {
          set_screen_status(e_screen_status_on);
        }
        break;
      case e_screen_status_off:
        if (millis() - _last_interaction < DELAY_DIMMING_DEFAULT) {
          set_screen_status(e_screen_status_on);
          clear();
          _screen->force_next_render();
          _menu->force_next_render();
          handle_input = false;
        }
        break;
    }

    if (_screen_status == e_screen_status_off) {
      return render_screensaver();
    }

    BaseScreen* new_screen = nullptr;
    if (handle_input && workers.any_updates()) {
      if (_menu->is_open()) {
        new_screen = _menu->handle_input(_controller, workers);
        if (new_screen || !_menu->is_open()) {
          // Closed menu
          DEBUG_PRINTLN("Menu closed");
          _menu->leave_screen(_controller);
          clear();
        }
      } else if (_screen) {
        new_screen = _screen->handle_input(_controller, workers);
        if (new_screen == _menu) {
          // opened menu, not a new screen
          DEBUG_PRINTLN("Menu opened");
          clear();
          _menu->enter_screen(_controller);
          new_screen = nullptr;
        } else if (new_screen == &DefaultEntryScreen_i) {
          // entered the default entry screen, handle it right away, no need to render this
          new_screen = new_screen->handle_input(_controller, workers);
        }
      }
      if (new_screen && new_screen != _screen) {
        DEBUG_PRINTF("New screen entered: %s\n", new_screen->get_title());
        _screen->leave_screen(_controller);
        _screen = new_screen;
        clear();
        _screen->enter_screen(_controller);
      }
    }

    if (workers.any_updates() || handlers.any_updates() || (millis() - _last_render > LCD_REFRESH_RATE)) {
      M5.Lcd.startWrite();
      M5.Lcd.setRotation(3);
      if (_menu->is_open()) {
        _menu->do_render(workers, handlers);
      } else {
        _screen->do_render(workers, handlers);
      }

      if (_screen->has_status_bar()) {
        // Render message if available on top of bar
        if (_screen->get_error_message(workers, handlers)) {
          M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_ERROR);
          auto text_width = M5.Lcd.drawString(_screen->get_error_message(workers, handlers), 0, 220, 2);
          M5.Lcd.fillRect(text_width, 200, 320 - text_width, 20, LCD_COLOR_BACKGROUND);
        } else if (_screen->get_status_message(workers, handlers)) {
          M5.Lcd.setTextColor(LCD_COLOR_BACKGROUND, LCD_COLOR_DEFAULT);
          auto text_width = M5.Lcd.drawString(_screen->get_status_message(workers, handlers), 0, 220, 2);
          M5.Lcd.fillRect(text_width, 200, 320 - text_width, 20, LCD_COLOR_BACKGROUND);
        } else {
          M5.Lcd.fillRect(0, 200, 320, 20, LCD_COLOR_BACKGROUND);
        }

        // Render bottom status bar
        M5.Lcd.drawLine(0, 220, 320, 220, TFT_WHITE);

        // Screen name
        M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
        M5.Lcd.setCursor(0, 227);
        M5.Lcd.print(_screen->get_title());
        M5.Lcd.print(" ");

        // Status icon: Battery
        const auto& battery = workers.worker<BatteryIndicator>(k_worker_battery_indicator)->get_data();
        M5.Lcd.setTextColor(battery.isCharging ? LCD_COLOR_ACTIVITY : LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
        M5.Lcd.printf("%d%% ", battery.percentage);

        // Status icon: Geiger Tube
        const auto& gm = workers.worker<GeigerCounter>(k_worker_gm_sensor);
        if (!gm->active()) {
          M5.Lcd.setTextColor(_screen->has_required_tube() ? LCD_COLOR_ERROR : LCD_COLOR_INACTIVE, TFT_BLACK);
        } else {
          M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
        }
        M5.Lcd.print("GM ");

        // Status icon: GPS
        const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);
        if (!gps->active()) {
          M5.Lcd.setTextColor(_screen->has_required_gps() ? LCD_COLOR_ERROR : LCD_COLOR_INACTIVE, TFT_BLACK);
          M5.Lcd.printf("GPS ");
        } else {
          M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_ACTIVITY : LCD_COLOR_STALE_INCOMPLETE, TFT_BLACK);
          M5.Lcd.printf("GPS%d ", gps->get_data().satsInView);
        }

        // Status icon: SD
        if (!SDInterface::i().can_write_logs()) {
          M5.Lcd.setTextColor(_screen->has_required_sd() ? LCD_COLOR_ERROR : LCD_COLOR_INACTIVE, TFT_BLACK);
        } else {
          M5.Lcd.setTextColor(SDInterface::i().just_wrote() ? LCD_COLOR_ACTIVITY : LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
        }
        M5.Lcd.print("SD ");

        // Status icon: Wi-Fi
        if (WiFiWrapper_i.wifi_connected()) {
          M5.Lcd.setTextColor(WiFiWrapper_i.was_active() ? LCD_COLOR_ACTIVITY : LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
        } else {
          M5.Lcd.setTextColor(_screen->has_required_wifi() ? LCD_COLOR_ERROR : LCD_COLOR_INACTIVE, LCD_COLOR_BACKGROUND);
        }
        M5.Lcd.print("WF ");

        // Status icon: Bluetooth
        M5.Lcd.setTextColor(_screen->has_required_ble() ? LCD_COLOR_ERROR : LCD_COLOR_INACTIVE, LCD_COLOR_BACKGROUND);
        M5.Lcd.print("BT ");

        // Device
        if (_settings.get_device_id() < 10000) {
          // 4-digit device id
          M5.Lcd.setCursor(186, 227);
          M5.Lcd.setTextColor(_settings.get_device_id() ? LCD_COLOR_DEFAULT : LCD_COLOR_ERROR, LCD_COLOR_BACKGROUND);
          M5.Lcd.printf("#%04d ", _settings.get_device_id());
        } else {
          // 5-digit device id
          M5.Lcd.setCursor(180, 227);
          M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
          M5.Lcd.printf("#%5d ", _settings.get_device_id());
        }

        // Time HH:MM
        const auto& rtc = workers.worker<RtcConnector>(k_worker_rtc_connector)->get_data();
        M5.Lcd.setTextColor(rtc.valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
        M5.Lcd.printf("%04d/%02d/%02d %02d:%02d", rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute);
      }

      M5.Lcd.setRotation(1);
      M5.Lcd.display();
      M5.Lcd.endWrite();
      _last_render = millis();

    }
  }

}

  void GFXScreen::render_screensaver() {
    if (_saver_enabled && millis() - _last_render > 75) {
      M5.Lcd.setRotation(3);
      if (_saver_x + _saver_x_direction < -2 || _saver_x + _saver_x_direction > 318 - SCREENSAVER_TEXT_LENGTH) {
        _saver_x_direction *= -1;
      }
      if (_saver_y + _saver_y_direction < -2 || _saver_y + _saver_y_direction > 230) {
        _saver_y_direction *= -1;
      }
      _saver_x += _saver_x_direction;
      _saver_y += _saver_y_direction;
      _saver.pushSprite(_saver_x, _saver_y, TFT_TRANSPARENT);
      M5.Lcd.setRotation(1);
      _last_render = millis();
    }
  }
