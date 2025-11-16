#include "config_mode.h"
#include "identifiers.h"
#include "menu_window.h"
#include "user_config.h"
#include "utils/device_utils.h"
#include "utils/error_beep.h"
#include "utils/power_manager.h"
#include "utils/wifi_connection.h"
#include "utils/sd_wrapper.h"
#include "workers/local_storage.h"
#include "workers/zen_button.h"
#include "workers/sound_manager.h"
#include "workers/gps_connector.h"
#include <WiFi.h>

const ConfigModeScreen::MenuItem CONFIG_MODE_MENU[ConfigModeScreen::e_config_MENU_MAX] = {
    {.title="View settings", .tooltip="View current device  settings", .enabled=true},
    {.title="Start Access Point", .tooltip="Start Wi-Fi access   point, connect with   pc or phone to       configure device", .enabled=true},
    {.title="Start on local", .tooltip="Connect to local     Wi-Fi, use pc or      phone on local       network to configure    device", .enabled=true},
    {.title="Load from SD", .tooltip="Read settings file     from the SD-card     and set to device", .enabled=true},
    {.title="Save to SD", .tooltip="Write current device      settings to the       SD-card config file", .enabled=true},
    {.title="Wipe SD Card", .tooltip="Delete all log files from the SD card", .enabled=true},
    {.title="Reset dose", .tooltip="Reset the accumulated dose rate to zero", .enabled=true},
    {.title="CPM Alert Level", .tooltip="Adjust the CPM alert threshold level", .enabled=true},
    {.title="Click Sound", .tooltip="Enable/disable Geiger click sounds", .enabled=true},
    {.title="Error Alert Sound", .tooltip="Enable/disable error alert beep sounds", .enabled=true},
    {.title="Audio Volume", .tooltip="Adjust global audio volume for clicks/alerts", .enabled=true},
    {.title="Dim Brightness", .tooltip="Adjust screen brightness when dimmed/screensaver", .enabled=true},
    {.title="Set Home GPS", .tooltip="Set current GPS      location as home     for Real Time mode", .enabled=true},
    {.title="Factory reset", .tooltip="Clear and reset      device and SD-card", .enabled=true},
    {.title="Back to main menu", .tooltip="Return to the main menu", .enabled=true},
};


ConfigModeScreen ConfigModeScreen_i;

ConfigModeScreen::ConfigModeScreen() : BaseScreenWithMenu("Settings", true), _main_page_info_section(0) {
}

void ConfigModeScreen::render_audio_volume_page(const worker_map_t& workers, const handler_map_t& handlers) {
  auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
  uint8_t current = settings->get_audio_volume();

  // Draw button indicators at bottom of screen
  drawButton1("-5%\nHold:-20%");
  drawButton2("+5%\nHold:+20%");
  drawButton3("Back");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 50, &fonts::Font2);
  M5.Lcd.printf("Audio Volume\n\n");
  M5.Lcd.printf("Current: %u%%\n\n", current);
  M5.Lcd.printf("Controls the global audio volume for\n");
  M5.Lcd.printf("clicks and CPM alert sounds.\n\n");
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Range: 0-100%%\n");
}

void ConfigModeScreen::render_dim_brightness_page(const worker_map_t& workers, const handler_map_t& handlers) {
  auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
  uint8_t current = settings->get_dim_brightness();

  // Draw button indicators at bottom of screen
  drawButton1("-5%\nHold:-20%");
  drawButton2("+5%\nHold:+20%");
  drawButton3("Back");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 50, &fonts::Font2);
  M5.Lcd.printf("Dim Brightness\n\n");
  M5.Lcd.printf("Current: %u%%\n\n", current);
  M5.Lcd.printf("Controls the screen brightness used\n");
  M5.Lcd.printf("when the device is DIM or the\n");
  M5.Lcd.printf("animated screensaver is active.\n\n");
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Range: 0-100%%\n");
}

BaseScreen* ConfigModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  if (menu_open()) {
    auto* new_screen = handle_menu_input(controller, workers, CONFIG_MODE_MENU, e_config_MENU_MAX);
    if (new_screen) {
      return new_screen;
    }
    // Handle menu selection
    if (_current_page == e_config_page_reset_dose) {
      auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
      settings->reset_dose_rate();
      
      // Clear screen and show restart message
      M5.Lcd.clear(LCD_COLOR_BACKGROUND);
      M5.Lcd.setRotation(3);
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.setCursor(46, 78, &fonts::Font4);  // Centered horizontally (320 - text width) / 2
      M5.Lcd.printf("DOSE RATE RESET\n");
      M5.Lcd.setCursor(100, 120, &fonts::Font2);
      M5.Lcd.printf("Restarting device...\n");
      
      delay(2000); // Wait 2 seconds
      DeviceUtils::shutdown(true); // Restart the device
    }
    else if (_current_page == e_config_page_back_to_main) {
      return &MenuWindow_i;
    }
  }
  else {
    auto button1 = workers.worker<ZenButton>(k_worker_button_1);
    auto button2 = workers.worker<ZenButton>(k_worker_button_2);
    auto button3 = workers.worker<ZenButton>(k_worker_button_3);
    if (button1->is_fresh() && button1->get_data().shortPress) {
      // Check if we're on the CPM threshold page for decrement functionality
      if (_current_page == e_config_page_cpm_threshold) {
        auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
        uint16_t current_threshold = settings->get_alert_threshold();
        uint16_t decrement = button1->get_data().longPress ? 100 : 10;
        uint16_t new_threshold;
        
        // Handle underflow protection
        if (current_threshold <= decrement || current_threshold - decrement < 10) {
          new_threshold = 10;  // Minimum threshold
        } else {
          new_threshold = current_threshold - decrement;
        }
        
        if (new_threshold != current_threshold) {
          settings->set_alert_threshold(new_threshold, false);
          
          // Save to SD card
          if (SDInterface::i().ready()) {
            SDInterface::i().write_safezen_file_from_settings(*settings, false);
          }
          
          force_next_render();
          M5_LOGD("CPM threshold decreased to: %u", new_threshold);
        }
      }
      // Check if we're on the click sound page for toggle functionality
      else if (_current_page == e_config_page_click_sound) {
        auto sound_manager = workers.worker<SoundManager>(k_worker_sound_manager);
        if (sound_manager) {
          bool new_state = sound_manager->toggleSound();
          // Persist confirmation already handled by SoundManager; also mirror to SD settings file
          auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
          if (SDInterface::i().ready()) {
            SDInterface::i().write_safezen_file_from_settings(*settings, false);
          }
          force_next_render();
          M5_LOGD("Click sound toggled to: %s", new_state ? "ENABLED" : "DISABLED");
        }
      }
      // Check if we're on the error alert sound page for toggle functionality
      else if (_current_page == e_config_page_error_alert_sound) {
        auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
        bool current_setting = settings->get_error_alert_sound();
        bool new_setting = !current_setting;
        
        settings->set_error_alert_sound(new_setting, false);
        
        // Save to SD card
        if (SDInterface::i().ready()) {
          SDInterface::i().write_safezen_file_from_settings(*settings, false);
        }
        
        force_next_render();
        M5_LOGD("Error alert sound toggled to: %s", new_setting ? "ENABLED" : "DISABLED");
      }
      // Check if we're on the dim brightness page for decrement functionality
      else if (_current_page == e_config_page_dim_brightness) {
        auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
        uint8_t current = settings->get_dim_brightness();
        uint8_t step = button1->get_data().longPress ? 20 : 5;
        uint8_t new_value = (current > step) ? (uint8_t)(current - step) : 0;

        if (new_value != current) {
          settings->set_dim_brightness(new_value, false);
          if (SDInterface::i().ready()) {
            SDInterface::i().write_safezen_file_from_settings(*settings, false);
          }
          force_next_render();
          M5_LOGD("Dim brightness decreased to: %u%%", new_value);
        }
      }
      else if (_current_page == e_config_page_audio_volume) {
        auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
        uint8_t current = settings->get_audio_volume();
        uint8_t step = button1->get_data().longPress ? 20 : 5;
        uint8_t new_value = (current > step) ? (uint8_t)(current - step) : 0;

        if (new_value != current) {
          settings->set_audio_volume(new_value, false);
          if (SDInterface::i().ready()) {
            SDInterface::i().write_safezen_file_from_settings(*settings, false);
          }
          force_next_render();
          M5_LOGD("Audio volume decreased to: %u%%", new_value);
        }
      }
      else {
        open_menu(true);
        M5.Lcd.clear(LCD_COLOR_BACKGROUND);  // Clear screen when opening menu
        force_next_render();
      }
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      // screen specific action
      switch (_current_page) {
        case e_config_page_reset_all:
          M5.Lcd.clear(LCD_COLOR_BACKGROUND);
          M5.Lcd.setRotation(3);
          M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
          M5.Lcd.setCursor(30, 78, &fonts::Font4);
          M5.Lcd.printf("RESET IN PROGRESS\n");
          M5.Lcd.setCursor(5, 120, &fonts::Font2);

          M5.Lcd.printf("Removing all log files, This can take a while...\n");
          controller.factory_reset();
          M5.Lcd.clear(LCD_COLOR_BACKGROUND);
          M5.Lcd.setRotation(3);
          M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
          M5.Lcd.setCursor(27, 78, &fonts::Font4);
          M5.Lcd.printf("DEVICE MEMORY AND\n");
          M5.Lcd.setCursor(27, 110, &fonts::Font4);
          M5.Lcd.printf("SD HAVE BEEN RESET\n");
          M5.Lcd.setCursor(70, 160, &fonts::Font2);
          M5.Lcd.printf("Please turn off the device...\n");
          delay(UINT32_MAX); // Sleep for a long time.
          break;
        case e_config_page_main:
          _main_page_info_section = (_main_page_info_section + 1) % e_config_section_MAX;
          force_next_render();
          break;
        case e_config_page_set_home_gps: {
          // Set current GPS location as home
          auto* gps = workers.worker<GpsConnector>(k_worker_gps_connector);
          auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
          
          if (gps && gps->active() && gps->get_data().location_valid) {
            double new_lat = gps->get_data().latitude;
            double new_lon = gps->get_data().longitude;
            
            // Save the new home location
            settings->set_fixed_latitude(new_lat, false);
            settings->set_fixed_longitude(new_lon, false);
            
            // Also update last known location for reference
            settings->set_last_latitude(new_lat, false);
            settings->set_last_longitude(new_lon, false);
            
            // Save to SD card
            if (SDInterface::i().ready()) {
              SDInterface::i().write_safezen_file_from_settings(*settings, false);
            }
            
            set_status_message(F(" HOME GPS LOCATION SET! "));
            force_next_render();
            M5_LOGD("Home GPS location set to: Lat %.6f, Lon %.6f", new_lat, new_lon);
          } else {
            set_status_message(F(" NO VALID GPS FIX! "));
            M5_LOGD("Cannot set home GPS - no valid fix");
          }
          break;
        }
        case e_config_page_cpm_threshold: {
          auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
          uint16_t current_threshold = settings->get_alert_threshold();
          uint16_t increment = button2->get_data().longPress ? 100 : 10;
          uint16_t new_threshold = current_threshold + increment;
          
          // Clamp to reasonable range
          if (new_threshold > 9999) {
            new_threshold = 9999;
          }
          if (new_threshold < 10) {
            new_threshold = 10;
          }
          
          if (new_threshold != current_threshold) {
            settings->set_alert_threshold(new_threshold, false);
            
            // Save to SD card
            if (SDInterface::i().ready()) {
              SDInterface::i().write_safezen_file_from_settings(*settings, false);
            }
            
            force_next_render();
            M5_LOGD("CPM threshold updated to: %u", new_threshold);
          }
          break;
        }
        case e_config_page_dim_brightness: {
          auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
          uint8_t current = settings->get_dim_brightness();
          uint8_t step = button2->get_data().longPress ? 20 : 5;
          uint16_t temp = current + step; // use wider type to avoid overflow before clamp
          uint8_t new_value = (temp > 100) ? 100 : (uint8_t)temp;

          if (new_value != current) {
            settings->set_dim_brightness(new_value, false);
            if (SDInterface::i().ready()) {
              SDInterface::i().write_safezen_file_from_settings(*settings, false);
            }
            force_next_render();
            M5_LOGD("Dim brightness increased to: %u%%", new_value);
          }
          break;
        }
        case e_config_page_audio_volume: {
          auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
          uint8_t current = settings->get_audio_volume();
          uint8_t step = button2->get_data().longPress ? 20 : 5;
          uint16_t temp = current + step; // use wider type to avoid overflow before clamp
          uint8_t new_value = (temp > 100) ? 100 : (uint8_t)temp;

          if (new_value != current) {
            settings->set_audio_volume(new_value, false);
            if (SDInterface::i().ready()) {
              SDInterface::i().write_safezen_file_from_settings(*settings, false);
            }
            force_next_render();
            M5_LOGD("Audio volume increased to: %u%%", new_value);
          }
          break;
        }
        default:
          break;
      }
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      // Handle page-specific back navigation
      switch (_current_page) {
        case e_config_page_cpm_threshold:
        case e_config_page_click_sound:
        case e_config_page_error_alert_sound:
        case e_config_page_dim_brightness:
        case e_config_page_audio_volume:
        case e_config_page_set_home_gps:
          // Go back to config menu, not main menu
          _current_page = e_config_page_main;
          M5.Lcd.clear(LCD_COLOR_BACKGROUND);  // Clear screen to avoid visual artifacts
          open_menu(false);
          force_next_render();
          return nullptr;  // Stay on config screen
        default:
          // For other pages, go back to main menu
          return &MenuWindow_i;
      }
    }
  }

  return nullptr;
}

void ConfigModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (menu_open()) {
    if (!force) {
      return;
    }
    render_menu(CONFIG_MODE_MENU, e_config_MENU_MAX, true, 1);
  }
  else {
    if (!force) {
      return;
    }
    clear_screen_content();

    switch (_current_page) {
      case e_config_page_main:
        render_page_main(workers, handlers);
        break;
      case e_config_page_ap:
        render_page_ap(workers, handlers);
        break;
      case e_config_page_wifi:
        render_page_wifi(workers, handlers);
        break;
      case e_config_page_sd_wipe:
        render_sd_wipe(workers, handlers);
        break;
      case e_config_page_cpm_threshold:
        render_cpm_threshold_page(workers, handlers);
        break;
      case e_config_page_click_sound:
        render_click_sound_page(workers, handlers);
        break;
      case e_config_page_error_alert_sound:
        render_error_alert_sound_page(workers, handlers);
        break;
      case e_config_page_dim_brightness:
        render_dim_brightness_page(workers, handlers);
        break;
      case e_config_page_audio_volume:
        render_audio_volume_page(workers, handlers);
        break;
      case e_config_page_set_home_gps:
        render_set_home_gps_page(workers, handlers);
        break;
      case e_config_page_reset_all:
        render_reset_device_sd(workers, handlers);
        break;
      default:
        break;
    }
  }
}

void ConfigModeScreen::render_page_main(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("Options");
  char button_text[13];
  sprintf(button_text, "More (%d/%d)", _main_page_info_section + 1, e_config_section_MAX);
  drawButton2(button_text);
  drawButton3("Menu");

  clear_screen_content();

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);

  // Temp print out device settings on screen
  const auto& config = *workers.worker<LocalStorage>(k_worker_local_storage);
  M5.Lcd.setCursor(0, 40, &fonts::Font2);

  if (_main_page_info_section == e_config_section_device) {
    M5.Lcd.printf("Device settings\n\n");
    M5.Lcd.printf("User name:   %s  \n", config.get_user_name());
    M5.Lcd.printf("Display unit:   %s  \n", config.get_cpm_usvh() ? "CPM" : "uSv/h");
    M5.Lcd.printf("Alert threshold:   %d  \n", config.get_alert_threshold());
    M5.Lcd.printf("Logging drive/survey:   %s  \n", config.get_manual_logging() ? "Manual start" : "Automatic");
    M5.Lcd.printf("Screen dim after:   %d seconds  \n", config.get_screen_dim_timeout());
    M5.Lcd.printf("Screen off after:   %d seconds  \n", config.get_screen_off_timeout());
    M5.Lcd.printf("Screensaver:   %s  \n", config.get_animated_screensaver() ? "Enabled" : "Disabled");
  }
  if (_main_page_info_section == e_config_section_location) {
    M5.Lcd.printf("Location settings\n\n");
    M5.Lcd.printf("Fixed longitude:   %0.6f  \n", config.get_fixed_longitude());
    M5.Lcd.printf("Fixed latitude:   %0.6f  \n", config.get_fixed_latitude());
    M5.Lcd.printf("Fixed range:   %0.2f km  \n", config.get_fixed_range());
    M5.Lcd.printf("Maximum valid DOP:   %u   \n", config.get_dop_max());
  }
  if (_main_page_info_section == e_config_section_connection) {
    M5.Lcd.printf("Connection settings\n\n");
    M5.Lcd.printf("AP password:   %s  \n", config.get_ap_password());
    M5.Lcd.printf("Profile 1 Wi-Fi ssid:   %s  \n", config.get_wifi_ssid());
    M5.Lcd.printf("Profile 1 Wi-Fi password:   %s  \n", config.get_wifi_password());
    M5.Lcd.printf("Profile 2 Wi-Fi ssid:   %s  \n", config.get_wifi_ssid2());
    M5.Lcd.printf("Profile 2 Wi-Fi password:   %s  \n", config.get_wifi_password2());
    M5.Lcd.printf("API key:   %s  \n", config.get_api_key());
  }

}

void ConfigModeScreen::render_page_ap(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("Options");
  drawButton3("Menu");

  const auto settings = workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setCursor(0, 70, &fonts::Font2);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Config through Access Point, connect to\n");
  M5.Lcd.printf("SSID:  %s\n", WiFiWrapper_i.get_hostname());
  M5.Lcd.printf("Password:  %s\n", settings->get_ap_password());

  M5.Lcd.printf("Connect to config page at url :  %s     \n", WiFi.softAPIP().toString().c_str());
}

void ConfigModeScreen::render_page_wifi(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("Options");
  drawButton3("Menu");

  const auto settings = workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setCursor(0, 70, &fonts::Font2);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Config through local network, connect to\n");
  M5.Lcd.printf("Network:  %s\n", settings->get_wifi_ssid());
  M5.Lcd.printf("Connected:  %s\n", WiFiWrapper_i.wifi_connected() ? "Yes          " : "Not yet...");

  M5.Lcd.printf("IP:  %s               \n", WiFiWrapper_i.wifi_connected() ? WiFi.localIP().toString().c_str() : "Waiting for connection... ");
}

void ConfigModeScreen::render_sd_wipe(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("Options");
  drawButton2("WIPE");
  drawButton3("Menu");

  // Create warning box with red background and white text
  playErrorBeepsIfAvailable(workers);
  M5.Lcd.fillRect(0, 70, 320, 60, LCD_COLOR_ERROR);
  M5.Lcd.setCursor(10, 85, &fonts::Font2);
  M5.Lcd.setTextColor(TFT_WHITE, LCD_COLOR_ERROR);
  M5.Lcd.printf("WARNING: This will delete ALL log files");
  M5.Lcd.setCursor(10, 105, &fonts::Font2);
  M5.Lcd.printf("from the SD card!");
  
  // Regular instructions below the warning
  M5.Lcd.setCursor(0, 150, &fonts::Font2);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Press WIPE to confirm deleting all log files.\n");
  M5.Lcd.printf("Your device settings will be preserved.\n");
}

void ConfigModeScreen::render_reset_device_sd(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("Options");
  drawButton2("RESET");
  drawButton3("Menu");

  const auto& storage = workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setCursor(0, 70, &fonts::Font2);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Press RESET to confirm clearing all local storage\n\n");
  M5.Lcd.printf("This will also clear all log files, and reset the \n");
  M5.Lcd.printf("settings on the SD-card to only contain your ID\n\n");
  M5.Lcd.printf("device ID: %d\n\n", storage->get_device_id());
}

void ConfigModeScreen::render_cpm_threshold_page(const worker_map_t& workers, const handler_map_t& handlers) {
  auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
  uint16_t current_threshold = settings->get_alert_threshold();
  
  // Draw button indicators at bottom of screen
  drawButton1("-10 CPM");
  drawButton2("+10 CPM");
  drawButton3("Back");
  
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 50, &fonts::Font2);
  M5.Lcd.printf("CPM Alert Threshold\n");
  M5.Lcd.printf("\n");
  M5.Lcd.printf("Current: %u CPM\n", current_threshold);
  M5.Lcd.printf("\n");
  M5.Lcd.printf("Press A to decrease by 10 CPM\n");
  M5.Lcd.printf("Press B to increase by 10 CPM\n");
  M5.Lcd.printf("Press C to go back\n");
  M5.Lcd.printf("\n");
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Hold A: -100 CPM\n");
  M5.Lcd.printf("Hold B: +100 CPM\n");
  M5.Lcd.printf("Range: 10-9999 CPM\n");
}

void ConfigModeScreen::render_click_sound_page(const worker_map_t& workers, const handler_map_t& handlers) {
  auto sound_manager = workers.worker<SoundManager>(k_worker_sound_manager);
  bool enabled = sound_manager && sound_manager->isSoundEnabled();
  
  // Draw button indicators at bottom of screen
  drawButton1("Toggle");
  drawButton2("");
  drawButton3("Back");
  
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 50, &fonts::Font2);
  M5.Lcd.printf("Click Sound\n");
  M5.Lcd.printf("\n");
  M5.Lcd.printf("Current: %s\n", enabled ? "ENABLED" : "DISABLED");
  M5.Lcd.printf("\n");
  M5.Lcd.printf("Controls Geiger counter click\n");
  M5.Lcd.printf("sound during normal operation.\n");
  
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("\n");
  M5.Lcd.printf("Tip: Long-press Menu Button\n");
  M5.Lcd.printf("toggles this too.\n");
}

void ConfigModeScreen::render_error_alert_sound_page(const worker_map_t& workers, const handler_map_t& handlers) {
  auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
  bool current_setting = settings->get_error_alert_sound();
  
  // Draw button indicators at bottom of screen
  drawButton1("Toggle");
  drawButton2("");
  drawButton3("Back");
  
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 50, &fonts::Font2);
  M5.Lcd.printf("Error Alert Sound\n");
  M5.Lcd.printf("\n");
  M5.Lcd.printf("Current: %s\n", current_setting ? "ENABLED" : "DISABLED");
  M5.Lcd.printf("\n");
  M5.Lcd.printf("Controls error beep sounds\n");
  M5.Lcd.printf("when system errors occur.\n");
  M5.Lcd.printf("\n");
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Note: CPM alert sounds are\n");
  M5.Lcd.printf("controlled separately.\n");
}

void ConfigModeScreen::render_set_home_gps_page(const worker_map_t& workers, const handler_map_t& handlers) {
  auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
  auto* gps = workers.worker<GpsConnector>(k_worker_gps_connector);
  
  // Draw button indicators at bottom of screen
  drawButton1("");
  drawButton2("Set Home");
  drawButton3("Back");
  
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 50, &fonts::Font2);
  M5.Lcd.printf("Set Home GPS Location\n\n");
  
  // Show current home location
  M5.Lcd.printf("Current Home (Real Time):\n");
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Lat: %.6f\n", settings->get_fixed_latitude());
  M5.Lcd.printf("Lon: %.6f\n", settings->get_fixed_longitude());
  
  // Show current GPS location if available
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

void ConfigModeScreen::enter_screen(Controller& controller) {
  // Ensure low power mode is disabled so that WiFi/AP will start reliably
  PowerManager::exitLowPowerMode();

  // If entering the Settings screen from outside and we're on the main page,
  // force the menu to open at the main entry
  if (_current_page == e_config_page_main) {
    _menu_index = e_config_page_main;
    open_menu(true);
    return;
  }

  // For other pages, perform page-specific actions when the page is entered
  switch (_current_page) {
    case e_config_page_ap:
      WiFiWrapper_i.start_ap_server(controller.get_settings().get_device_id(), controller.get_settings().get_ap_password());
      controller.set_worker_active(k_worker_config_server, true);
      break;
    case e_config_page_wifi:
      WiFiWrapper_i.connect_wifi(controller.get_settings().get_wifi_ssid(), controller.get_settings().get_wifi_password());
      controller.set_worker_active(k_worker_config_server, true);
      break;
    case e_config_page_load_sd_config:
      if (controller.load_sd_config()) {
        set_status_message(F(" SD CONFIG LOADED, settings have been updated! "));
      }
      else {
        set_status_message(F(" LOAD CONFIG FAILED, no config or SD-card! "));
      }
      _current_page = e_config_page_main; // Back on main page
      open_menu(true);       // Re-enter menu
      break;
    case e_config_page_save_config_to_sd:
      if (controller.write_sd_config()) {
        set_status_message(F(" CONFIG SAVED, settings have been saved to SD! "));
      }
      else {
        set_status_message(F(" WRITE CONFIG FAILED, no SD-card! "));
      }
      _current_page = e_config_page_main; // Back on main page
      open_menu(true);       // Re-enter menu
      break;
    default:
      break;
  }
}

void ConfigModeScreen::leave_screen(Controller& controller) {
  switch (_current_page) {
    case e_config_page_ap:
      WiFiWrapper_i.stop_ap_server();
      controller.set_worker_active(k_worker_config_server, false);
      break;
    case e_config_page_wifi:
      WiFiWrapper_i.disconnect_wifi();
      controller.set_worker_active(k_worker_config_server, false);
      break;
    default:
      break;
  }

  // Set page to main in case we left through main menu
  _current_page = e_config_page_main;
  _main_page_info_section = e_config_section_device;
}
