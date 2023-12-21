#include "fixed_mode.h"
#include "identifiers.h"
#include "menu_window.h"
#include "workers/zen_button.h"

FixedModeScreen FixedModeScreen_i;

FixedModeScreen::FixedModeScreen() : BaseScreen("Fixed", true) {
}

BaseScreen* FixedModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return &MenuWindow_i;
  }
  return nullptr;
}

void FixedModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  drawButton3("Menu");
}

void FixedModeScreen::enter_screen(Controller& controller) {
}

void FixedModeScreen::leave_screen(Controller& controller) {
}