#include "fixed_mode.h"
#include "handlers/api_connector.h"
#include "identifiers.h"
#include "menu_window.h"
#include "user_config.h"
#include "utils/error_beep.h"
#include "workers/gm_sensor.h"
#include "workers/gps_connector.h"
#include "workers/local_storage.h"
#include "workers/log_aggregator.h"
#include "workers/zen_button.h"
#include <esp_wifi.h>
#include "utils/power_manager.h"

FixedModeScreen FixedModeScreen_i;

FixedModeScreen::FixedModeScreen() : BaseScreen("Real-time", true) {
  required_gps = true;
  required_tube = true;
  required_wifi = true;
  required_sd = true;
}

BaseScreen* FixedModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  // Handle Profile button (Button 2)
  auto profile_button = workers.worker<ZenButton>(k_worker_button_2);
  if (profile_button->is_fresh() && profile_button->get_data().shortPress) {
    const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);
    uint8_t current = settings->get_wifi_profile_active();
    uint8_t next = current == 1 ? 2 : 1;
    settings->set_wifi_profile_active(next, true);
    // Restart Wi-Fi with new credentials
    WiFiWrapper_i.disconnect_wifi();
    WiFiWrapper_i.connect_wifi(settings->get_active_wifi_ssid(), settings->get_active_wifi_password(), true);
    // Show quick feedback
    set_status_message(next == 1 ? F(" PROFILE 1 ") : F(" PROFILE 2 "));
    force_next_render();
  }

  // Handle Menu button (Button 3)
  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return &MenuWindow_i;
  }
  return nullptr;
}

void FixedModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  // Display Profile and Menu buttons
  char profile_label[14];
  sprintf(profile_label, "Profile %u", workers.worker<LocalStorage>(k_worker_local_storage)->get_wifi_profile_active());
  drawButton2(profile_label);
  drawButton3("Menu");

  const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);
  const auto& gm_sensor = workers.worker<GeigerCounter>(k_worker_gm_sensor);
  const auto& log_aggregator = workers.worker<LogAggregator>(k_worker_log_aggregator);
  const auto& api_connector = handlers.handler<ApiConnector>(k_handler_api_reporter);


  if (gm_sensor->is_fresh() || force) {
    // Auto-switch units based on value magnitude
    uint32_t cpm_value = gm_sensor->get_data().cpm_comp;
    float dose_value = gm_sensor->get_data().uSvh;
    const char* cpm_unit = " CPM";
    const char* dose_unit = " uSv/h";
    
    // Convert to KCPM if > 1000
    float display_cpm = cpm_value;
    if (cpm_value > 1000) {
      display_cpm = cpm_value / 1000.0f;
      cpm_unit = " KCPM";
    }
    
    // Convert to mSv/h if > 1000
    float display_dose = dose_value;
    if (dose_value > 1000) {
      display_dose = dose_value / 1000.0f;
      dose_unit = " mSv/h";
    }
    
    // Limit display to prevent overlap with QR code
    // Stricter limits for fixed mode with QR code
    bool use_scientific = false;
    
    // For dose values, limit based on magnitude to keep total width under control
    if (dose_value > 1000) {
      if (display_dose > 999.99f) {
        display_dose = 999.99f;
        dose_unit = "+mSv/h";
      }
    } else if (dose_value > 999.99f) {
      // Convert high uSv/h to mSv/h to save space
      display_dose = dose_value / 1000.0f;
      dose_unit = " mSv/h";
    }
    
    // For KCPM values > 999.99, limit display
    if (cpm_value > 1000 && display_cpm > 999.99f) {
      display_cpm = 999.99f;  // Cap at max displayable value
      cpm_unit = "+KCPM";  // Add + to indicate overflow
    }
    
    if (settings->get_cpm_usvh()) {
      // Display CPM big, usvh small
      M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      
      // Always display exactly 3 digits for clean appearance
      uint16_t cpm_width;
      if (cpm_value >= 1000) {
        // Convert to KCPM for values >= 1000
        float kcpm = cpm_value / 1000.0f;
        if (kcpm > 999.0f) {
          cpm_width = printIntFont(999, 0, 100, &fonts::Font7);
          cpm_unit = "+KCPM";
        } else if (kcpm >= 100.0f) {
          cpm_width = printIntFont((int)kcpm, 0, 100, &fonts::Font7);  // xxx format
          cpm_unit = " KCPM";
        } else if (kcpm >= 10.0f) {
          cpm_width = printFloatFont(kcpm, 1, 0, 100, &fonts::Font7);  // xx.x format
          cpm_unit = " KCPM";
        } else {
          cpm_width = printFloatFont(kcpm, 2, 0, 100, &fonts::Font7);  // x.xx format
          cpm_unit = " KCPM";
        }
      } else {
        // Display as CPM (0-999)
        cpm_width = printIntFont(cpm_value, 0, 100, &fonts::Font7);
        cpm_unit = " CPM";
      }
      
      uint16_t ush_width = printFloatFont(display_dose, (dose_value > 1000) ? 2 : 4, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (CPM/KCPM uSv/h/mSv/h)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(cpm_width, 52, 220 - cpm_width, 27, LCD_COLOR_BACKGROUND); // Prints blanks after cpm value, above CPM text
      cpm_width += M5.Lcd.drawString(cpm_unit, cpm_width, 105, &fonts::Font4); // Prints after cpm value
      M5.Lcd.fillRect(cpm_width, 74, 220 - cpm_width, 26, LCD_COLOR_BACKGROUND); // Prints blanks after CPM text
      M5.Lcd.drawString((String(dose_unit) + "   ").c_str(), 0 + ush_width, 140, &fonts::Font4); // Prints after ush value
    } else {
      M5.Lcd.setTextColor(gm_sensor->get_data().valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      
      // Always display exactly 3 digits for clean appearance
      uint16_t ush_width;
      if (dose_value >= 1000.0f) {
        // Convert to mSv/h for values >= 1000 uSv/h
        float msv = dose_value / 1000.0f;
        if (msv > 999.0f) {
          ush_width = printIntFont(999, 0, 100, &fonts::Font7);
          dose_unit = "+mSv/h";
        } else if (msv >= 100.0f) {
          ush_width = printIntFont((int)msv, 0, 100, &fonts::Font7);  // xxx format
          dose_unit = " mSv/h";
        } else if (msv >= 10.0f) {
          ush_width = printFloatFont(msv, 1, 0, 100, &fonts::Font7);  // xx.x format
          dose_unit = " mSv/h";
        } else {
          ush_width = printFloatFont(msv, 2, 0, 100, &fonts::Font7);  // x.xx format
          dose_unit = " mSv/h";
        }
      } else if (dose_value >= 100.0f) {
        // Display as integer uSv/h (100-999)
        ush_width = printIntFont((int)dose_value, 0, 100, &fonts::Font7);  // xxx format
        dose_unit = " uSv/h";
      } else if (dose_value >= 10.0f) {
        ush_width = printFloatFont(dose_value, 1, 0, 100, &fonts::Font7);  // xx.x format
        dose_unit = " uSv/h";
      } else {
        ush_width = printFloatFont(dose_value, 2, 0, 100, &fonts::Font7);  // x.xx format
        dose_unit = " uSv/h";
      }
      
      uint16_t cpm_width = (cpm_value > 1000)
        ? printFloatFont(display_cpm, 2, 0, 140, &fonts::Font4)
        : printIntFont(cpm_value, 0, 140, &fonts::Font4);

      // Display unit text with cleanup (CPM/KCPM uSv/h/mSv/h)
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.fillRect(ush_width, 52, 220 - ush_width, 27, LCD_COLOR_BACKGROUND); // Prints blanks after dose value, above dose text
      ush_width += M5.Lcd.drawString(dose_unit, ush_width, 105, &fonts::Font4); // Prints after dose value
      M5.Lcd.fillRect(ush_width, 74, 220 - ush_width, 26, LCD_COLOR_BACKGROUND); // Prints blanks after dose text
      M5.Lcd.drawString((String(cpm_unit) + "   ").c_str(), 0 + cpm_width, 140, &fonts::Font4); // Prints after cpm value
    }
  }

  if (api_connector->get_status() == ApiConnector::e_handler_processing) {
    set_status_message(F(" SENDING... "));
  }

  if (api_connector->get_status() == ApiConnector::e_api_reporter_send_success) {
    set_status_message(F(" SENT DATA TO API! "));
  }


  // Display GPS data always, change colour if not fresh
  if (log_aggregator->is_fresh() || force) {
    const auto& log_data = log_aggregator->get_data();
    // Print drive data
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);

    // Print fixed setting data
    M5.Lcd.setCursor(0, 150);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Home latitude  :");
    M5.Lcd.printf("%0.6f  ", settings->get_fixed_latitude());
    M5.Lcd.setCursor(0, 159);
    M5.Lcd.print("Home longitude :");
    M5.Lcd.printf("%0.6f  ", settings->get_fixed_longitude());
    M5.Lcd.setCursor(0, 168);
    M5.Lcd.print("You are        :");

    if (log_data.gps_valid) {
      M5.Lcd.setTextColor(log_data.in_fixed_range ? LCD_COLOR_DEFAULT : LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);
      M5.Lcd.printf(log_data.in_fixed_range ? "At home    " : "Roaming    ");
    } else {
      // Check if GPS hardware is present vs completely missing
      const auto& gps = workers.worker<GpsConnector>(k_worker_gps_connector);
      if (!gps->active()) {
        // GPS hardware not present - this is a real error, play beeps
        setErrorColorWithBeep(workers);
      } else {
        // GPS hardware present but no valid fix - don't play error beeps
        M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
      }
      M5.Lcd.printf("Without GPS");
    }


    M5.Lcd.setCursor(0, 177);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("total uploads  :%d", api_connector->get_post_count());

    // Print location data
    M5.Lcd.setCursor(170, 150);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Latitude   :");
    M5.Lcd.setTextColor(log_data.gps_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.6f  ", log_data.latitude);
    M5.Lcd.setCursor(170, 159);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Longitude  :");
    M5.Lcd.setTextColor(log_data.gps_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.6f  ", log_data.longitude);
    M5.Lcd.setCursor(170, 168);
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.print("Altitude   :");
    M5.Lcd.setTextColor(log_data.gps_valid ? LCD_COLOR_DEFAULT : LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
    M5.Lcd.printf("%0.2f    ", log_data.altitude);
  }

  if (force) {
    // Display QR on the side
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    char qr_url[130];
    const auto& config = workers.worker<LocalStorage>(k_worker_local_storage);
    sprintf(qr_url, FIXED_MODE_GRAFANA_URL, config->get_fixed_device_id(), config->get_fixed_device_id());
    M5.Lcd.qrcode(qr_url, 220, 40, 90);
  }

}

void FixedModeScreen::enter_screen(Controller& controller) {
  controller.set_handler_active(k_handler_api_reporter, true);

  // Enter low power mode (CPU/I2C down, but WiFi stays on)
  // Note: Core2 runs at 80MHz always (set at boot), CoreS3 reduces to 80MHz here
  PowerManager::enterLowPowerMode(false);

  // --- WiFi Power Save Mode and TX Power (Core2 specific) ---
  // For ESP32 (Core2), we need to be more careful with WiFi power management
  // Only apply power save settings after ensuring WiFi is stable
  #ifndef CONFIG_IDF_TARGET_ESP32S3
    // Core2 (ESP32) - Apply conservative power settings to avoid WiFi init loops
    // Wait a bit for power management to stabilize
    delay(100);
    
    // Only apply power save if WiFi is already connected or connecting
    if (WiFi.status() == WL_CONNECTED || WiFi.status() == WL_DISCONNECTED) {
      esp_wifi_set_ps(WIFI_PS_NONE);     // Disable power save for Core2 stability
      esp_wifi_set_max_tx_power(20);     // Use moderate TX power (5 dBm)
    }
  #else
    // CoreS3 (ESP32-S3) - Can handle more aggressive power saving
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM); // Enable minimum modem power save
    esp_wifi_set_max_tx_power(15);      // Set TX power to 15 (units: 0.25 dBm, so 15 = 3.75 dBm)
  #endif
  // ----------------------------------------
}

void FixedModeScreen::leave_screen(Controller& controller) {
  controller.set_handler_active(k_handler_api_reporter, false);

  // Restore normal power settings
  PowerManager::exitLowPowerMode();

  // --- Restore WiFi Power Settings ---
  esp_wifi_set_ps(WIFI_PS_NONE);      // Disable WiFi power save
  esp_wifi_set_max_tx_power(78);      // Restore TX power to max (78 * 0.25 = 19.5 dBm)
  // -----------------------------------
}