#include "first_time_startup.h"
#include "default_entry_screen.h"
#include "identifiers.h"
#include "user_config.h"
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
    return DefaultEntryScreen::i();
  }
  return nullptr;
}

void FirstTimeStartupScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  drawButton1("More info", e_button_disabled);
  drawButton3("Continue");

  const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setTextColor(LCD_COLOR_ACTIVE, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("Welcome to your Zen!", 38, 80, 4);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("#", 135, 100, 2);
  M5.Lcd.drawNumber(settings->get_device_id(), 145, 100, 2);
  M5.Lcd.drawString("Your device is now ready for use.", 55, 130, 2);
  M5.Lcd.drawString("Have fun measuring!", 100, 160, 2);
  M5.Lcd.drawString("- Safecast team", 110, 180, 2);
}

void FirstTimeStartupScreen::enter_screen(Controller& controller) {
  controller.load_sd_config();
}