#include "config_mode.h"
#include "workers/zen_button.h"
#include "identifiers.h"
#include "menu_window.h"

ConfigModeScreen::ConfigModeScreen(): BaseScreen("Config", true) {

}

BaseScreen* ConfigModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return MenuWindow::i();
  }
  return nullptr;
}

void ConfigModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton3("Menu");
}

void ConfigModeScreen::enter_screen(Controller& controller) {
}

void ConfigModeScreen::leave_screen(Controller& controller) {
}