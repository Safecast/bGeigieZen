#include "boot_screen.h"
#include "controller.h"
#include "default_entry_screen.h"
#include "first_time_startup.h"
#include "identifiers.h"
#include "sd_message.h"
#include "user_config.h"
<<<<<<< HEAD
#include "workers/rtc_connector.h"

#ifdef VERSION_BETA
#define BOOT_VERSION_STRING VERSION_STRING " beta"
#else
#define BOOT_VERSION_STRING VERSION_STRING
#endif
=======
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416

BootScreen BootScreen_i;

BootScreen::BootScreen() : BaseScreen("Boot", false), _entered_at(0) {
}

BaseScreen* BootScreen::handle_input(Controller& controller, const worker_map_t& workers) {
<<<<<<< HEAD
  if (millis() > 6000 + _entered_at) {
=======
  if (millis() > 3000 + _entered_at) {
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
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

  const auto& storage = workers.worker<LocalStorage>(k_worker_local_storage);
<<<<<<< HEAD
  const auto& rtc = workers.worker<DateTimeProvider>(k_worker_rtc_connector);

  // Display title
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("bGeigie Zen", 95, 50, &fonts::Font4);
  
  // Display version prominently
  M5.Lcd.setTextColor(LCD_COLOR_ACTIVITY, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString(BOOT_VERSION_STRING, 95, 85, &fonts::Font2);
  
  // Display user info
=======

  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("bGeigie Zen", 95, 50, &fonts::Font4);
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(5, 125);
  M5.Lcd.setFont(&fonts::Font2);
  M5.Lcd.printf("User name: %s", storage->get_user_name());
  M5.Lcd.setCursor(5, 143);
  M5.Lcd.printf("Device id: %d", storage->get_device_id());
<<<<<<< HEAD
  
  // Display date and time if available
  if (rtc && rtc->is_fresh()) {
    const auto& dt = rtc->get_data();
    M5.Lcd.setCursor(5, 163);
    M5.Lcd.printf("Date: %04d-%02d-%02d", dt.year, dt.month, dt.day);
    M5.Lcd.setCursor(5, 183);
    M5.Lcd.printf("Time: %02d:%02d:%02d", dt.hour, dt.minute, dt.second);
  }
=======
  M5.Lcd.setCursor(5, 163);
  M5.Lcd.printf("Version: %s", VERSION_SIMPLE_STRING);
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416

  // Display safecast copyright
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("SAFECAST", 230, 215, &fonts::Font0);
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString(COPYRIGHT_YEAR_STRING, 285, 215, &fonts::Font0);
}
