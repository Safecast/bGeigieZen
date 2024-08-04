#include "menu_window.h"
#include "config_mode.h"
#include "controller.h"
#include "debug_mode.h"
#include "drive_mode.h"
#include "fixed_mode.h"
#include "identifiers.h"
#include "satellite_view.h"
#include "survey_mode.h"
#include "user_config.h"
#include "workers/zen_button.h"
#include "zen_info.h"



const MenuWindow::MenuItem MAIN_MENU_ITEMS[MAIN_MENU_MAX] = {
    {.title="Drive mode", .tooltip="Put the zen on your car and drive!", .enabled=true, .screen=&DriveModeScreen_i},
    {.title="Survey mode", .tooltip="Take the Zen out of the case and test   sources directly!", .enabled=true, .screen=&SurveyModeScreen_i},
    {.title="Real-time mode", .tooltip="Place the zen at a    fixed location or    take it with you and    upload data real-   time over wifi!", .enabled=true, .screen=&FixedModeScreen_i},
    {.title="Satellites", .tooltip="A 2d constellation   map for viewing and    configuring satellites", .enabled=true, .screen=&SatelliteViewScreen_i},
    {.title="Log viewer", .tooltip="View and upload     logs over wifi!", .enabled=false, .screen=nullptr},
    {.title="Settings", .tooltip="Configure your      device!", .enabled=true, .screen=&ConfigModeScreen_i},
    {.title="About Zen", .tooltip="Explore what you    can do with your     bGeigieZen", .enabled=true, .screen=&ZenInfoScreen_i},
    {.title="Debug info", .tooltip="Connected modules, their data and status all in a simple view", .enabled=true, .screen=&DebugModeScreen_i}
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
