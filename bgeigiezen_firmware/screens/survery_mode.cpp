#include "survey_mode.h"
#include "workers/zen_button.h"
#include "identifiers.h"
#include "menu_window.h"

SurveyModeScreen::SurveyModeScreen(): BaseScreen("Survey", true) {

}

BaseScreen* SurveyModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return MenuWindow::i();
  }
  return nullptr;
}

void SurveyModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton3("Menu");
}

void SurveyModeScreen::enter_screen(Controller& controller) {
}

void SurveyModeScreen::leave_screen(Controller& controller) {
}