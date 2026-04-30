#include "utilities.h"
#include "config_mode.h"
#include "controller.h"
#include "debug_mode.h"
#include "identifiers.h"
#include "user_config.h"
#include "utils/device_utils.h"
#include "workers/local_storage.h"
#include "workers/zen_button.h"

UtilitiesScreen UtilitiesScreen_i;

static const UtilitiesScreen::MenuItem UTILITIES_MENU[UtilitiesScreen::e_utilities_MENU_MAX] = {
    {.title="Debug info", .tooltip="View connected modules and their status", .enabled=true, .screen=&DebugModeScreen_i},
    {.title="Factory reset", .tooltip="Clear and reset device and SD-card", .enabled=true, .screen=nullptr},
    {.title="Reset dose", .tooltip="Reset the accumulated dose rate to zero", .enabled=true, .screen=nullptr},
    {.title="About this device", .tooltip="Show device, location and connection info", .enabled=true, .screen=nullptr},
    {.title="Back", .tooltip="Return to Settings", .enabled=true, .screen=&ConfigModeScreen_i},
};

UtilitiesScreen::UtilitiesScreen() : BaseScreenWithMenu("Utilities", true), _info_section(0) {
  _current_page = e_utilities_MENU_MAX;  // sentinel: "show menu, no action page"
}

void UtilitiesScreen::enter_screen(Controller& controller) {
  if (_current_page == e_utilities_MENU_MAX) {
    // External entry — open the top of the submenu
    _menu_index = 0;
    _info_section = 0;
    open_menu(true);
  }
  // else: handle_menu_input did an internal page-swap; leave _current_page alone so render() shows the page
  force_next_render();
}

void UtilitiesScreen::leave_screen(Controller& controller) {
  _current_page = e_utilities_MENU_MAX;
}

BaseScreen* UtilitiesScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  if (menu_open()) {
    auto* new_screen = handle_menu_input(controller, workers, UTILITIES_MENU, e_utilities_MENU_MAX);
    if (new_screen) {
      return new_screen;
    }
    // Reset dose has no dedicated page — perform action immediately and restart
    if (_current_page == e_utilities_page_reset_dose) {
      auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
      settings->reset_dose_rate();

      M5.Lcd.clear(LCD_COLOR_BACKGROUND);
      M5.Lcd.setRotation(3);
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.setCursor(46, 78, &fonts::Font4);
      M5.Lcd.printf("DOSE RATE RESET\n");
      M5.Lcd.setCursor(100, 120, &fonts::Font2);
      M5.Lcd.printf("Restarting device...\n");

      delay(2000);
      DeviceUtils::shutdown(true);
    }
    return nullptr;
  }

  auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);

  if (_current_page == e_utilities_page_factory_reset) {
    if (button2->is_fresh() && button2->get_data().shortPress) {
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
      delay(UINT32_MAX);
    }
  } else if (_current_page == e_utilities_page_info) {
    if (button2->is_fresh() && button2->get_data().shortPress) {
      _info_section = (_info_section + 1) % e_info_section_MAX;
      force_next_render();
    }
  }

  if (button3->is_fresh() && button3->get_data().shortPress) {
    open_menu(true);
    M5.Lcd.clear(LCD_COLOR_BACKGROUND);
    force_next_render();
  }
  return nullptr;
}

void UtilitiesScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!force) {
    return;
  }
  if (menu_open()) {
    render_menu(UTILITIES_MENU, e_utilities_MENU_MAX, true, 1);
    return;
  }
  clear_screen_content();
  switch (_current_page) {
    case e_utilities_page_factory_reset:
      render_factory_reset_page(workers, handlers);
      break;
    case e_utilities_page_info:
      render_info_page(workers, handlers);
      break;
    default:
      break;
  }
}

void UtilitiesScreen::render_factory_reset_page(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("");
  drawButton2("RESET");
  drawButton3("Back");

  const auto& storage = workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setCursor(0, 70, &fonts::Font2);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Press RESET to confirm clearing all local storage\n\n");
  M5.Lcd.printf("This will also clear all log files, and reset the \n");
  M5.Lcd.printf("settings on the SD-card to only contain your ID\n\n");
  M5.Lcd.printf("device ID: %d\n\n", storage->get_device_id());
}

void UtilitiesScreen::render_info_page(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("");
  char button_text[13];
  sprintf(button_text, "More (%d/%d)", _info_section + 1, e_info_section_MAX);
  drawButton2(button_text);
  drawButton3("Back");

  const auto& config = *workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 40, &fonts::Font2);

  if (_info_section == e_info_section_device) {
    M5.Lcd.printf("Device settings\n\n");
    M5.Lcd.printf("User name:   %s  \n", config.get_user_name());
    M5.Lcd.printf("Device ID:   %d  \n", config.get_device_id());
    M5.Lcd.printf("Display unit:   %s  \n", config.get_cpm_usvh() ? "CPM" : "uSv/h");
    M5.Lcd.printf("Alert threshold:   %d  \n", config.get_alert_threshold());
    M5.Lcd.printf("Logging:   %s  \n", config.get_manual_logging() ? "Manual" : "Automatic");
    M5.Lcd.printf("Dim after:   %d s  \n", config.get_screen_dim_timeout());
    M5.Lcd.printf("Off after:   %d s  \n", config.get_screen_off_timeout());
    M5.Lcd.printf("Screensaver:   %s  \n", config.get_animated_screensaver() ? "Enabled" : "Disabled");
  }
  if (_info_section == e_info_section_location) {
    M5.Lcd.printf("Location settings\n\n");
    M5.Lcd.printf("Fixed longitude:   %0.6f  \n", config.get_fixed_longitude());
    M5.Lcd.printf("Fixed latitude:   %0.6f  \n", config.get_fixed_latitude());
    M5.Lcd.printf("Fixed range:   %0.2f km  \n", config.get_fixed_range());
    M5.Lcd.printf("Maximum valid DOP:   %u   \n", config.get_dop_max());
  }
  if (_info_section == e_info_section_connection) {
    M5.Lcd.printf("Connection settings\n\n");
    M5.Lcd.printf("AP password:   %s  \n", config.get_ap_password());
    M5.Lcd.printf("Profile 1 SSID:   %s  \n", config.get_wifi_ssid());
    M5.Lcd.printf("Profile 1 password:   %s  \n", config.get_wifi_password());
    M5.Lcd.printf("Profile 2 SSID:   %s  \n", config.get_wifi_ssid2());
    M5.Lcd.printf("Profile 2 password:   %s  \n", config.get_wifi_password2());
    M5.Lcd.printf("API key:   %s  \n", config.get_api_key());
  }
}
