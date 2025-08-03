#include <M5Unified.hpp>

#include "controller.h"
#include "gfx_screen.h"
#include "handlers/bluetooth_reporter.h"
#include "utils/error_beep.h"
#include "identifiers.h"
#include "screens/boot_screen.h"
#include "screens/default_entry_screen.h"
#include "screens/drive_mode.h"
#include "screens/fixed_mode.h"
#include "screens/satellite_view.h"
#include "screens/survey_mode.h"
#include "utils/wifi_connection.h"
#include "workers/battery_indicator.h"
#include "workers/gm_sensor.h"
#include "workers/rtc_connector.h"
#include "workers/zen_button.h"
#include "workers/sound_manager.h"

#define SCREENSAVER_TEXT_LENGTH (strlen(SCREENSAVER_TEXT) * 6)
#define TIMEOUT_PASSED(timeout, last_interaction) (timeout && (millis() - last_interaction) > (timeout * 1000))
static constexpr uint8_t LEVEL_BRIGHT = 100;  // Full brightness for active use (100%)
static constexpr uint8_t LEVEL_DIMMED = 20;   // 20% brightness for dimmed state (more noticeable difference)
static constexpr uint8_t LEVEL_BLANKED = 5;    // 5% brightness when blanked


GFXScreen::GFXScreen(LocalStorage& settings, Controller& controller)
    : Supervisor(),
      _saver(&M5.Lcd),
      _controller(controller),
      _settings(settings),
      _last_render(0),
      _last_interaction(0),
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

  _saver.createSprite(static_cast<int16_t>(4 + SCREENSAVER_TEXT_LENGTH), 12);
  _saver.fillScreen(LCD_COLOR_BACKGROUND);
  _saver.drawString(SCREENSAVER_TEXT, 2, 2, &fonts::Font0);
  _screen = &BootScreen_i;
  _screen->enter_screen(_controller);
  set_screen_status(e_screen_status_on);
  clear();
}

void GFXScreen::set_screen_status(ScreenStatus status) {
  static ScreenStatus last_status = static_cast<ScreenStatus>(255);
  
  if (status != last_status) {
    const char* status_str = "";
    switch (status) {
      case e_screen_status_on: status_str = "ON"; break;
      case e_screen_status_dim: status_str = "DIM"; break;
      case e_screen_status_off: status_str = "OFF"; break;
    }
    Serial.printf("Screen status changed from %d to %s\n", last_status, status_str);
    last_status = status;
  }
  
  _screen_status = status;
  switch (_screen_status) {
    case e_screen_status_on:
      setBrightness(LEVEL_BRIGHT);
      break;
    case e_screen_status_dim:
      setBrightness(LEVEL_DIMMED);
      break;
    case e_screen_status_off:
      clear();
      if (_settings.get_animated_screensaver()) {
        setBrightness(LEVEL_DIMMED);
      } else {
        setBrightness(LEVEL_BLANKED);
      }
      break;
  }
}

//setup brightness by Rob Oudendijk 2023-03-13
void GFXScreen::setBrightness(uint8_t lvl) {
  static uint8_t last_lvl = 255; // Initialize to impossible value
  
  // Only print if brightness level changes
  if (lvl != last_lvl) {
    Serial.printf("Setting brightness to: %d (was: %d)\n", lvl, last_lvl);
    last_lvl = lvl;
  }

  // For M5Stack CoreS3, use M5.Lcd.setBrightness
  // Map 0-100 range to 0-255 range with a non-linear curve for better visibility
  uint8_t mapped_brightness;
  if (lvl == LEVEL_BRIGHT) {
    mapped_brightness = 255;  // Full brightness
  } else if (lvl == LEVEL_DIMMED) {
    mapped_brightness = 64;   // 25% of max for dimmed state
  } else {
    mapped_brightness = 12;   // ~5% for blanked state
  }
  
  // Apply the brightness
  M5.Lcd.setBrightness(mapped_brightness);
  
  // Force update the display
  M5.Lcd.wakeup();
}

void GFXScreen::clear() {
  M5.Lcd.startWrite();
  
  // Set background color based on screen status
  if (_screen_status == e_screen_status_dim || _screen_status == e_screen_status_off) {
    M5.Lcd.fillScreen(TFT_BLACK);  // Use black background when dimmed or off
  } else {
    M5.Lcd.clear();  // Use default background for normal operation
  }
  
  M5.Lcd.setTextDatum(BL_DATUM);  // By default, text x,y is bottom left corner
  M5.Lcd.setTextFont(1);
  
  if (_screen) {
    _screen->force_next_render();
  }
  if (_menu) {
    _menu->force_next_render();
  }
  
  M5.Lcd.endWrite();
}

void GFXScreen::handle_report(const worker_map_t& workers, const handler_map_t& handlers) {
  if (_screen) {

    // Keep track of screen interaction / wake up
    const auto button1 = workers.worker<ZenButton>(k_worker_button_1);
    const auto button2 = workers.worker<ZenButton>(k_worker_button_2);
    const auto button3 = workers.worker<ZenButton>(k_worker_button_3);

    if (button1->is_fresh() || button2->is_fresh() || button3->is_fresh() || (M5.Touch.isEnabled() && M5.Touch.getCount())) {
      _last_interaction = millis();
    }

    // Will be set false when returning from wake up
    bool handle_input = true;

    switch (_screen_status) {
      case e_screen_status_on:
        if (TIMEOUT_PASSED(_settings.get_screen_dim_timeout(), _last_interaction)) {
          set_screen_status(e_screen_status_dim);
        }
        else if (TIMEOUT_PASSED(_settings.get_screen_off_timeout(), _last_interaction)) {
          set_screen_status(e_screen_status_off);
        }
        break;
      case e_screen_status_dim:
        if (millis() - _last_interaction < (LCD_REFRESH_RATE + 1000)) {
          set_screen_status(e_screen_status_on);
        }
        else if (TIMEOUT_PASSED(_settings.get_screen_off_timeout(), _last_interaction)) {
          set_screen_status(e_screen_status_off);
        }
        break;
      case e_screen_status_off:
        if (millis() - _last_interaction < (LCD_REFRESH_RATE + 1000)) {
          clear();
          set_screen_status(e_screen_status_on);
          handle_input = false;
        }
        break;
    }

    if (_screen_status == e_screen_status_off) {
      return render_screensaver();
    }

    BaseScreen* new_screen = nullptr;
    if (handle_input && workers.any_updates()) {
      if (_menu->menu_open()) {
        new_screen = _menu->handle_input(_controller, workers);
        if (new_screen || !_menu->menu_open()) {
          // Closed menu
          M5_LOGD("Menu closed");
          _menu->leave_screen(_controller);
          clear();
        }
      } else if (_screen) {
        new_screen = _screen->handle_input(_controller, workers);
        if (new_screen == _menu) {
          // opened menu, not a new screen
          M5_LOGD("Menu opened");
          clear();
          _menu->enter_screen(_controller);
          new_screen = nullptr;
        } else if (new_screen == &DefaultEntryScreen_i) {
          // entered the default entry screen, handle it right away, no need to render this
          new_screen = new_screen->handle_input(_controller, workers);
        }
      }
      if (new_screen && new_screen != _screen) {
        M5_LOGD("New screen entered: %s", new_screen->get_title());
        _screen->leave_screen(_controller);
        _screen = new_screen;
        clear();
        _screen->enter_screen(_controller);

        // Check if operational mode and save
        if (new_screen == &DriveModeScreen_i) {
          _settings.set_last_mode(LocalStorage::e_operational_mode_drive, false);
        }
        if (new_screen == &SurveyModeScreen_i) {
          _settings.set_last_mode(LocalStorage::e_operational_mode_survey, false);
        }
        if (new_screen == &FixedModeScreen_i) {
          _settings.set_last_mode(LocalStorage::e_operational_mode_fixed, false);
        }
        if (new_screen == &SatelliteViewScreen_i) {
          _settings.set_last_mode(LocalStorage::e_operational_mode_satellite, false);
        }

      }
    }

    if (workers.any_updates() || handlers.any_updates() || (millis() - _last_render > LCD_REFRESH_RATE)) {
      M5.Lcd.startWrite();
      M5.Lcd.setRotation(3);
      if (_menu->menu_open()) {
        _menu->do_render(workers, handlers);
      } else {
        _screen->do_render(workers, handlers);
      }

      if (_screen->has_status_bar()) {
        // Only update status bar at a reduced rate to prevent flickering
        static unsigned long last_status_bar_update = 0;
        static bool status_bar_needs_full_redraw = true;
        static bool message_displayed = false;
        static String last_error_message = "";
        static String last_status_message = "";
        static unsigned long message_display_time = 0;
        static const unsigned long MESSAGE_TIMEOUT = 5000; // 5 seconds timeout for messages
        static BaseScreen* last_error_screen = nullptr; // Track which screen showed the error
        static String last_shown_error = ""; // Track the last error message that was actually shown
        
        // Define message area properties based on Font2 (height 16px) and current baseline 220
        const int MSG_AREA_Y_BASELINE = 220;
        const int MSG_AREA_FONT_HEIGHT = 16; // M5.Lcd.fontHeight(&fonts::Font2) is 16
        const int MSG_AREA_Y_TOP = MSG_AREA_Y_BASELINE - MSG_AREA_FONT_HEIGHT;
        const int MSG_AREA_HEIGHT = MSG_AREA_FONT_HEIGHT;
        
        // Get current messages from the screen
        const __FlashStringHelper* error_msg = _screen->get_error_message(workers, handlers);
        const __FlashStringHelper* status_msg = _screen->get_status_message(workers, handlers);
        
        String current_error_message = error_msg ? String(error_msg) : "";
        String current_status_message = status_msg ? String(status_msg) : "";
        
        // Check if messages have changed
        bool message_changed = (current_error_message != last_error_message) || (current_status_message != last_status_message);
        last_error_message = current_error_message;
        last_status_message = current_status_message;
        
        // Check for message timeout
        bool message_timed_out = message_displayed && (millis() - message_display_time > MESSAGE_TIMEOUT);
        
        // Check if we should show error message (new screen or different error)
        bool should_show_error = error_msg && (
          (_screen != last_error_screen) || // New screen
          (current_error_message != last_shown_error) || // Different error message
          (!message_displayed && last_shown_error.isEmpty()) // First time showing any error
        );
        
        // Handle message display
#ifdef M5STACK_CORE2
        // Core2 message handling
        if (should_show_error) {
          // Error message takes precedence - only show on screen change or new error
          playErrorBeepsIfAvailable(workers);
          M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_ERROR);
          uint16_t text_width = M5.Lcd.drawString(current_error_message.c_str(), 0, MSG_AREA_Y_BASELINE, &fonts::Font2);
          M5.Lcd.fillRect(text_width, MSG_AREA_Y_TOP, M5.Lcd.width() - text_width, MSG_AREA_HEIGHT, LCD_COLOR_BACKGROUND);
          message_displayed = true;
          message_display_time = millis();
          last_error_screen = _screen;
          last_shown_error = current_error_message;
          status_bar_needs_full_redraw = true;
        } else if (status_msg && (!message_displayed || current_status_message != last_status_message)) {
          // Status message - only show on new message or first display
          M5.Lcd.setTextColor(LCD_COLOR_BACKGROUND, LCD_COLOR_DEFAULT);
          uint16_t text_width = M5.Lcd.drawString(current_status_message.c_str(), 0, MSG_AREA_Y_BASELINE, &fonts::Font2);
          M5.Lcd.fillRect(text_width, MSG_AREA_Y_TOP, M5.Lcd.width() - text_width, MSG_AREA_HEIGHT, LCD_COLOR_BACKGROUND);
          message_displayed = true;
          message_display_time = millis();
          status_bar_needs_full_redraw = true;
        } else if (message_displayed && (message_timed_out || (!error_msg && !status_msg))) {
          // Clear message area if timed out or no message and there was one before
          M5.Lcd.fillRect(0, MSG_AREA_Y_TOP, M5.Lcd.width(), MSG_AREA_HEIGHT, LCD_COLOR_BACKGROUND);
          message_displayed = false;
          status_bar_needs_full_redraw = true;
          // Don't clear last_shown_error here - keep it to prevent re-showing on same screen
        }
#else
        // CoreS3 specific message handling
        if (should_show_error) {
          // Error message takes precedence - only show on screen change or new error
          playErrorBeepsIfAvailable(workers);
          M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_ERROR);
          uint16_t text_width = M5.Lcd.drawString(current_error_message.c_str(), 0, MSG_AREA_Y_BASELINE, &fonts::Font2);
          M5.Lcd.fillRect(text_width, MSG_AREA_Y_TOP, M5.Lcd.width() - text_width, MSG_AREA_HEIGHT, LCD_COLOR_BACKGROUND);
          message_displayed = true;
          message_display_time = millis();
          last_error_screen = _screen;
          last_shown_error = current_error_message;
          status_bar_needs_full_redraw = true;
        } else if (status_msg && (!message_displayed || current_status_message != last_status_message)) {
          // Status message - only show on new message or first display
          M5.Lcd.setTextColor(LCD_COLOR_BACKGROUND, LCD_COLOR_DEFAULT);
          uint16_t text_width = M5.Lcd.drawString(current_status_message.c_str(), 0, MSG_AREA_Y_BASELINE, &fonts::Font2);
          M5.Lcd.fillRect(text_width, MSG_AREA_Y_TOP, M5.Lcd.width() - text_width, MSG_AREA_HEIGHT, LCD_COLOR_BACKGROUND);
          message_displayed = true;
          message_display_time = millis();
          status_bar_needs_full_redraw = true;
        } else if (message_displayed && (message_timed_out || (!error_msg && !status_msg))) {
          // Clear message area if timed out or no message and there was one before
          M5.Lcd.fillRect(0, MSG_AREA_Y_TOP, M5.Lcd.width(), MSG_AREA_HEIGHT, LCD_COLOR_BACKGROUND);
          message_displayed = false;
          status_bar_needs_full_redraw = true;
          // Don't clear last_shown_error here - keep it to prevent re-showing on same screen
        }
#endif

        // Only update the status bar every 1000ms or if a full redraw is needed
        if (millis() - last_status_bar_update > 1000 || status_bar_needs_full_redraw || message_changed) {
          last_status_bar_update = millis();
          status_bar_needs_full_redraw = false;
          
          // Clear the entire status bar area at once
          M5.Lcd.fillRect(0, 221, 320, 19, LCD_COLOR_BACKGROUND);
          
          // Render bottom status bar
          M5.Lcd.drawLine(0, 220, 320, 220, TFT_WHITE);
          M5.Lcd.setFont(&fonts::Font0);
          
          // Screen name
          M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
          M5.Lcd.setCursor(0, 235);
          M5.Lcd.print(_screen->get_title());
          
          // Calculate positions for status indicators
          int pos = 60; // Starting position after screen name (reduced from 60)
          
          // Status icon: Battery
          M5.Lcd.setCursor(pos, 235);
          const auto& battery = workers.worker<BatteryIndicator>(k_worker_battery_indicator)->get_data();
          M5.Lcd.setTextColor(battery.isCharging ? LCD_COLOR_ACTIVITY : LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
          M5.Lcd.printf("%d%%", battery.percentage);
          pos += 28; 

          // Status icon: Geiger Tube
          M5.Lcd.setCursor(pos, 235);
          const auto& gm = workers.worker<GeigerCounter>(k_worker_gm_sensor);
          if (!gm->active()) {
            M5.Lcd.setTextColor(_screen->has_required_tube() ? LCD_COLOR_ERROR : LCD_COLOR_INACTIVE, TFT_BLACK);
          } else {
            M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
          }
          M5.Lcd.print("GM");
          pos += 16; 

          // Status icon: GPS
          M5.Lcd.setCursor(pos, 235);
          const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);
          if (!gps->active()) {
            M5.Lcd.setTextColor(_screen->has_required_gps() ? LCD_COLOR_ERROR : LCD_COLOR_INACTIVE, TFT_BLACK);
            M5.Lcd.print("GPS");
            pos += 30; 
          } else {
            M5.Lcd.setTextColor(gps->get_data().location_valid ? LCD_COLOR_ACTIVITY : LCD_COLOR_STALE_INCOMPLETE, TFT_BLACK);
            M5.Lcd.printf("GPS%d", gps->get_data().satsInView);
            pos += 34; 
          }

          // Status icon: SD
          M5.Lcd.setCursor(pos, 235);
          if (!SDInterface::i().can_write_logs()) {
            bool sd_error_state = _screen->has_required_sd();
            M5.Lcd.setTextColor(sd_error_state ? LCD_COLOR_ERROR : LCD_COLOR_INACTIVE, TFT_BLACK);
            
            // Play beeps when switching to a screen that requires SD card but none is available
            static BaseScreen* last_screen = nullptr;
            if (sd_error_state && _screen != last_screen) {
              playErrorBeepsIfAvailable(workers);
            }
            last_screen = _screen;
          } else {
            M5.Lcd.setTextColor(SDInterface::i().just_wrote() ? LCD_COLOR_ACTIVITY : LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
            static BaseScreen* last_screen = nullptr;
            last_screen = _screen; // Update screen tracking even when SD is working
          }
          M5.Lcd.print("SD");
          pos += 18; // Adjust position for next indicator

          // Status icon: Wi-Fi
          M5.Lcd.setCursor(pos, 235);
          if (WiFiWrapper_i.wifi_connected()) {
            M5.Lcd.setTextColor(WiFiWrapper_i.was_active() ? LCD_COLOR_ACTIVITY : LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
          } else {
            M5.Lcd.setTextColor(_screen->has_required_wifi() ? LCD_COLOR_ERROR : LCD_COLOR_INACTIVE, LCD_COLOR_BACKGROUND);
          }
          M5.Lcd.print("WF");
          pos += 18; // Adjust position for next indicator
          
          // Status icon: Bluetooth
          M5.Lcd.setCursor(pos, 235);
          const auto& bt_reporter = handlers.handler<BluetoothReporter>(k_handler_bluetooth_reporter);
          if (bt_reporter->active()) {
            M5.Lcd.setTextColor(bt_reporter->client_count() > 0 ? LCD_COLOR_ACTIVITY : LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
          } else {
            M5.Lcd.setTextColor(_screen->has_required_ble() ? LCD_COLOR_ERROR : LCD_COLOR_INACTIVE, LCD_COLOR_BACKGROUND);
          }
          M5.Lcd.print("BT");
          pos += 18; // Adjust position for next indicator
          
          // Status icon: Sound
          extern SoundManager sound_manager;
          M5.Lcd.setCursor(pos, 235);
          if (sound_manager.isSoundEnabled()) {
            M5.Lcd.setTextColor(LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);
          } else {
            M5.Lcd.setTextColor(LCD_COLOR_INACTIVE, TFT_BLACK);
          }
          M5.Lcd.print("SN");
          pos += 16; // Adjust position for next indicator

          // Device
          M5.Lcd.setCursor(pos, 235);
          if (_settings.get_device_id() < 10000) {
            // 4 digits
            M5.Lcd.setTextColor(_settings.get_device_id() ? LCD_COLOR_DEFAULT : LCD_COLOR_ERROR, LCD_COLOR_BACKGROUND);
            M5.Lcd.printf("#%04d", _settings.get_device_id());
          } else {
            // 5-digit device id
            M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
            M5.Lcd.printf("#%5d", _settings.get_device_id());
          }
          pos += 40; // Adjust position for next indicator

          // Date
          const auto& rtc = workers.worker<DateTimeProvider>(k_worker_rtc_connector)->get_data();
          M5.Lcd.setCursor(pos, 235);
          M5.Lcd.setTextColor(rtc.valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
          M5.Lcd.printf("%02d/%02d %02d:%02d", rtc.month, rtc.day, rtc.hour, rtc.minute);
        }
      }

      M5.Lcd.setRotation(1);
      M5.Lcd.display();
      M5.Lcd.endWrite();
      _last_render = millis();

    }
  }

}

void GFXScreen::render_screensaver() {
  if (_settings.get_animated_screensaver() && millis() - _last_render > 75) {
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
