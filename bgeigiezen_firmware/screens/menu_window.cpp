#include "menu_window.h"
#include "flight_mode.h"
#include "config_mode.h"
#include "controller.h"
#include "debug_mode.h"
#include "drive_mode.h"
#include "fixed_mode.h"
#include "identifiers.h"
#include "log_viewer.h"
#include "satellite_view.h"
#include "sd_wipe.h"
#include "survey_mode.h"
#include "user_config.h"
#include "workers/zen_button.h"
#include "zen_info.h"



const MenuWindow::MenuItem MAIN_MENU_ITEMS[MAIN_MENU_MAX] = {
    {.title="Drive mode", .tooltip="Log radiation data with GPS", .enabled=true, .screen=&DriveModeScreen_i},
    {.title="Survey mode", .tooltip="Log radiation data to SD card", .enabled=true, .screen=&SurveyModeScreen_i},
    {.title="Real Time mode", .tooltip="Real-time upload to API", .enabled=true, .screen=&FixedModeScreen_i},
    {.title="Cosmic mode", .tooltip="Log data with optimized power settings", .enabled=true, .screen=&FlightModeScreen_i},
    {.title="Satellite view", .tooltip="A 2d constellation map for viewing satellites", .enabled=true, .screen=&SatelliteViewScreen_i},
    {.title="Log viewer", .tooltip="Log viewer (in progress)", .enabled=false, .screen=&LogViewerScreen_i},
    {.title="Settings", .tooltip="Configure your device", .enabled=true, .screen=&ConfigModeScreen_i},
    {.title="About Zen", .tooltip="Explore what you can do with your bGeigieZen", .enabled=true, .screen=&ZenInfoScreen_i},
    {.title="Debug info", .tooltip="View connected modules and their status", .enabled=true, .screen=&DebugModeScreen_i}
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
  open_menu(true);
  force_next_render();
}

void MenuWindow::leave_screen(Controller& controller) {
  open_menu(false);
}
