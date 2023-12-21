#include "boot_screen.h"
#include "controller.h"
#include "debugger.h"
#include "default_entry_screen.h"
#include "first_time_startup.h"
#include "identifiers.h"
#include "sd_message.h"
#include "user_config.h"

BootScreen BootScreen_i;

BootScreen::BootScreen() : BaseScreen("Boot", false), _entered_at(0) {
}

BaseScreen* BootScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  DEBUG_PRINTF("[%lu] boot screen handle input, is done: %d\n", millis(), millis() > 3000 + _entered_at);
  if (millis() > 3000 + _entered_at) {
    const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);
    if (controller.get_data().sd_card_status != SDInterface::SdStatus::e_sd_config_status_ok) {
      DEBUG_PRINTF("[%lu] boot screen handled input, enter SdMessageScreen\n", millis());
      return &SdMessageScreen_i;
    } else if (!settings->get_device_id()) {
      DEBUG_PRINTF("[%lu] boot screen handled input, enter FirstTimeStartupScreen\n", millis());
      return &FirstTimeStartupScreen_i;
    }
    DEBUG_PRINTF("[%lu] boot screen handled input, enter DefaultEntryScreen\n", millis());
    return &DefaultEntryScreen_i;
  }
  return nullptr;
}

void BootScreen::enter_screen(Controller& controller) {
  _entered_at = millis();
}

void BootScreen::leave_screen(Controller& controller) {
}

void BootScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  // Display something
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("bGeigie Zen", 90, 50, 4);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("Some splash screen device details.", 5, 100, 2);
  M5.Lcd.drawString(VERSION_STRING, 5, 120, 2);
  // Display safecast copyright
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("SAFECAST", 230, 215, 1);
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("2023", 285, 215, 1);
}
