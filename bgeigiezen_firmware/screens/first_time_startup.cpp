#include "first_time_startup.h"
#include "drive_mode.h"
#include "identifiers.h"
#include "workers/zen_button.h"

FirstTimeStartupScreen::FirstTimeStartupScreen() : BaseScreen("Welcome", false) {
}

BaseScreen* FirstTimeStartupScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto info_button = workers.worker<ZenButton>(k_worker_button_1);
  auto continue_button = workers.worker<ZenButton>(k_worker_button_3);
  if (info_button->is_fresh() && info_button->get_data().shortPress) {
    //    return Drive::i();
  }
  if (continue_button->is_fresh() && continue_button->get_data().shortPress) {
    return DriveModeScreen::i();
  }
  return nullptr;
}

void FirstTimeStartupScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  drawButton3("More info");
  drawButton3("Continue");
}

void FirstTimeStartupScreen::enter_screen(Controller& controller) {
}

void FirstTimeStartupScreen::leave_screen(Controller& controller) {
}