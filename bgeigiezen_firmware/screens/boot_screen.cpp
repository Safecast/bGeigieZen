#include "boot_screen.h"
#include "controller.h"
#include "default_entry_screen.h"
#include "first_time_startup.h"
#include "identifiers.h"
#include "sd_message.h"
#include "user_config.h"

BootScreen BootScreen_i;

BootScreen::BootScreen() : BaseScreen("Boot", false), _entered_at(0) {
}

BaseScreen* BootScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  if (millis() > 3000 + _entered_at) {
    const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);
    if (controller.get_data().sd_card_status != SDInterface::SdStatus::e_sd_config_status_ok) {
      return &SdMessageScreen_i;
    } else if (!settings->get_device_id()) {
      return &FirstTimeStartupScreen_i;
    } else {
      return &DefaultEntryScreen_i;
    }
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
  M5.Lcd.drawString("bGeigie Zen", 90, 50, &fonts::Font4);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("Some splash screen device details.", 5, 100, &fonts::Font2);
  M5.Lcd.drawString(VERSION_STRING, 5, 120, &fonts::Font2);
  // Display safecast copyright
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("SAFECAST", 230, 215, &fonts::Font0);
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString(COPYRIGHT_YEAR_STRING, 285, 215, &fonts::Font0);
}
