#include "config_mode.h"
#include "debugger.h"
#include "identifiers.h"
#include "menu_window.h"
#include "user_config.h"
#include "utils/device_utils.h"
#include "utils/wifi_connection.h"
#include "workers/local_storage.h"
#include "workers/zen_button.h"
#include <WiFi.h>

const ConfigModeScreen::MenuItem CONFIG_MODE_MENU[ConfigModeScreen::e_config_MENU_MAX] = {
    {.title="View settings", .tooltip="View current device  settings", .enabled=true},
    {.title="Start Access Point", .tooltip="Start Wi-Fi access   point, connect with   pc or phone to       configure device", .enabled=true},
    {.title="Start on local", .tooltip="Connect to local     Wi-Fi, use pc or      phone on local       network to configure    device", .enabled=true},
    {.title="Load from SD", .tooltip="Read settings file     from the SD-card     and set to device", .enabled=true},
    {.title="Save to SD", .tooltip="Write current device      settings to the       SD-card config file", .enabled=true},
    {.title="Reset device", .tooltip="Clear and reset      device, on reboot it      will load initial       settings from SD-card", .enabled=true},
};


ConfigModeScreen ConfigModeScreen_i;

ConfigModeScreen::ConfigModeScreen() : BaseScreenWithMenu("Settings", true), _main_page_info_section(0) {
}

BaseScreen* ConfigModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {

  if (menu_open()) {
    return handle_menu_input(controller, workers, CONFIG_MODE_MENU, e_config_MENU_MAX);
  }
  else {
    auto button1 = workers.worker<ZenButton>(k_worker_button_1);
    auto button2 = workers.worker<ZenButton>(k_worker_button_2);
    auto button3 = workers.worker<ZenButton>(k_worker_button_3);
    if (button1->is_fresh() && button1->get_data().shortPress) {
      open_menu(true);
      M5.Lcd.clear();
      force_next_render();
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      // screen specific action
      switch (_current_page) {
        case e_config_page_reset:
          controller.reset_all();
          // Temp clear and post reset message to screen
          M5.Lcd.clear(LCD_COLOR_BACKGROUND);
          M5.Lcd.setRotation(3);
          M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
          M5.Lcd.setCursor(17, 78, 4);
          M5.Lcd.printf("DEVICE MEMORY RESET\n");
          M5.Lcd.setCursor(100, 120, 2);
          M5.Lcd.printf("Restarting device...\n");
          delay(1000);
          DeviceUtils::shutdown(true);
          break;
        case e_config_page_main:
          _main_page_info_section = (_main_page_info_section + 1) % e_config_section_MAX;
          force_next_render();
        default:
          break;
      }
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      return &MenuWindow_i;
    }
  }

  return nullptr;
}

void ConfigModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {

  if (menu_open()) {
    if (!force) {
      return;
    }
    return render_menu(CONFIG_MODE_MENU, e_config_MENU_MAX);
  }

  switch (_current_page) {
    case e_config_page_main:
      if (!force) {
        return;
      }
      return render_page_main(workers, handlers);
    case e_config_page_ap:
      return render_page_ap(workers, handlers);
    case e_config_page_wifi:
      return render_page_wifi(workers, handlers);
    case e_config_page_reset:
      return render_reset_device(workers, handlers);
    default:
      return;
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
  M5.Lcd.setCursor(0, 30, 2);

  if (_main_page_info_section == e_config_section_device) {
    M5.Lcd.printf("Device settings\n\n");
    M5.Lcd.printf("Display unit:   %s  \n", config.get_cpm_usvh() ? "CPM" : "uSv/h");
    M5.Lcd.printf("Alert threshold:   %d  \n", config.get_alert_threshold());
    M5.Lcd.printf("Logging drive/survey:   %s  \n", config.get_manual_logging() ? "Manual start" : "Automatic");
    M5.Lcd.printf("Measurements in log file:   %s  \n", config.get_log_void() ? "Valid and invalid" : "Only valid");
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
    M5.Lcd.printf("local Wi-Fi ssid:   %s  \n", config.get_wifi_ssid());
    M5.Lcd.printf("local Wi-Fi password:   %s  \n", config.get_wifi_password());
    M5.Lcd.printf("API key:   %s  \n", config.get_api_key());
  }

}

void ConfigModeScreen::render_page_ap(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("Options");
  drawButton3("Menu");

  const auto settings = workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setCursor(0, 78, 1);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Config through Access Point, connect to\n");
  M5.Lcd.printf("SSID:        %s\n", WiFiWrapper_i.get_hostname());
  M5.Lcd.printf("Password:    %s\n", settings->get_ap_password());

  M5.Lcd.printf("IP:          %s     \n", WiFi.softAPIP().toString().c_str());
}

void ConfigModeScreen::render_page_wifi(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("Options");
  drawButton3("Menu");

  const auto settings = workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setCursor(0, 78, 1);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Config through local network, connect to\n");
  M5.Lcd.printf("Network:     %s\n", settings->get_wifi_ssid());
  M5.Lcd.printf("Connected:   %s\n", WiFiWrapper_i.wifi_connected() ? "Yes       " : "Not yet...");

  M5.Lcd.printf("IP:          %s            \n", WiFiWrapper_i.wifi_connected() ? WiFi.localIP().toString().c_str() : "Waiting for connection... ");
}

void ConfigModeScreen::render_reset_device(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("Options");
  drawButton2("RESET");
  drawButton3("Menu");

  M5.Lcd.setCursor(0, 78, 1);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Press RESET to confirm clearing all local storage\n");
}

void ConfigModeScreen::enter_screen(Controller& controller) {
  switch (_current_page) {
    case e_config_page_main:
      // In case when entering from main menu, always set config menu index correctly
      _menu_index = e_config_page_main;
      break;
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
