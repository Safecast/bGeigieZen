#include "config_mode.h"
#include "display_settings.h"
#include "gps_settings.h"
#include "menu_window.h"
#include "sd_settings.h"
#include "sound_settings.h"
#include "utilities.h"
#include "utils/power_manager.h"
#include "wifi_settings.h"

static const ConfigModeScreen::MenuItem CONFIG_MODE_MENU[ConfigModeScreen::e_config_MENU_MAX] = {
    {.title="Display", .tooltip="Brightness and screen behaviour", .enabled=true, .screen=&DisplaySettingsScreen_i},
    {.title="GPS", .tooltip="Home location and GPS settings", .enabled=true, .screen=&GpsSettingsScreen_i},
    {.title="Sound", .tooltip="Volume, clicks, alarm and CPM threshold", .enabled=true, .screen=&SoundSettingsScreen_i},
    {.title="SD card", .tooltip="Load/save settings and wipe log files", .enabled=true, .screen=&SdSettingsScreen_i},
    {.title="WiFi", .tooltip="Access Point or local network configuration", .enabled=true, .screen=&WifiSettingsScreen_i},
    {.title="Utilities", .tooltip="Debug info, factory reset, reset dose, device information", .enabled=true, .screen=&UtilitiesScreen_i},
    {.title="Back to main menu", .tooltip="Return to the main menu", .enabled=true, .screen=&MenuWindow_i},
};

ConfigModeScreen ConfigModeScreen_i;

ConfigModeScreen::ConfigModeScreen() : BaseScreenWithMenu("Settings", true) {
}

void ConfigModeScreen::enter_screen(Controller& controller) {
  // Ensure low power mode is disabled so the submenu screens that need WiFi/AP can run
  PowerManager::exitLowPowerMode();

  // Always land on the top-level Settings menu
  open_menu(true);
  force_next_render();
}

BaseScreen* ConfigModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  return handle_menu_input(controller, workers, CONFIG_MODE_MENU, e_config_MENU_MAX);
}

void ConfigModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!force) {
    return;
  }
  render_menu(CONFIG_MODE_MENU, e_config_MENU_MAX, true, 1);
}
