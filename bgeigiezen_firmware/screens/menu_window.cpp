#include "menu_window.h"
#include "flight_mode.h"
#include "config_mode.h"
#include "controller.h"
#include "drive_mode.h"
#include "fixed_mode.h"
#include "log_viewer.h"
#include "satellite_view.h"
#include "survey_mode.h"
#include "workers/local_storage.h"
#include "usb_transfer_screen.h"

const MenuWindow::MenuItem MAIN_MENU_ITEMS[MAIN_MENU_MAX] = {
    {.title="Drive mode", .tooltip="Log radiation data with GPS", .enabled=true, .screen=&DriveModeScreen_i},
    {.title="Survey mode", .tooltip="Log radiation data to SD card", .enabled=true, .screen=&SurveyModeScreen_i},
    {.title="Real Time mode", .tooltip="Real-time upload to API", .enabled=true, .screen=&FixedModeScreen_i},
    {.title="Cosmic mode", .tooltip="Log data with optimized power settings", .enabled=true, .screen=&FlightModeScreen_i},
    {.title="Satellite view", .tooltip="A 2d constellation map for viewing satellites", .enabled=true, .screen=&SatelliteViewScreen_i},
    {.title="Log viewer", .tooltip="View and upload SD log files over WiFi", .enabled=true, .screen=&LogViewerScreen_i},
    {.title="Settings", .tooltip="Configure your device. Debug info and device information are under Settings > Utilities.", .enabled=true, .screen=&ConfigModeScreen_i},
    {.title="USB File Transfer", .tooltip="Transfer SD card files via USB-C connection", .enabled=true, .screen=&USBTransferScreen_i}
};

MenuWindow MenuWindow_i;

MenuWindow::MenuWindow() : BaseScreenWithMenu("Menu", true) {
}

BaseScreen* MenuWindow::handle_input(Controller& controller, const worker_map_t& workers) {
  return handle_menu_input(controller, workers, MAIN_MENU_ITEMS, MAIN_MENU_MAX);
}

void MenuWindow::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!menu_open() || !force) {
    return;
  }
  render_menu(MAIN_MENU_ITEMS, MAIN_MENU_MAX, true, 3);
}

void MenuWindow::enter_screen(Controller& controller) {
  // Restore selection to the last-used operational mode so the menu
  // doesn't always start on Drive after a power cycle.
  switch (controller.get_settings().get_last_mode()) {
    case LocalStorage::e_operational_mode_drive:     _menu_index = 0; break;
    case LocalStorage::e_operational_mode_survey:    _menu_index = 1; break;
    case LocalStorage::e_operational_mode_fixed:     _menu_index = 2; break;
    case LocalStorage::e_operational_mode_flight:    _menu_index = 3; break;
    case LocalStorage::e_operational_mode_satellite: _menu_index = 4; break;
  }
  open_menu(true);
  force_next_render();
}

void MenuWindow::leave_screen(Controller& controller) {
  open_menu(false);
}
