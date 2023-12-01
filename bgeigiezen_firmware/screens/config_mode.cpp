#include "config_mode.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/zen_button.h"

ConfigModeScreen::ConfigModeScreen() : BaseScreen("Settings", true) {
}

BaseScreen* ConfigModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return MenuWindow::i();
  }
  auto reset_button = workers.worker<ZenButton>(k_worker_button_1);
  if (reset_button->is_fresh() && reset_button->get_data().shortPress) {
    controller.reset_all();
  }
  return nullptr;
}

void ConfigModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  drawButton1("Reset device");
  drawButton3("Menu");
}

void ConfigModeScreen::enter_screen(Controller& controller) {
}

void ConfigModeScreen::leave_screen(Controller& controller) {
}