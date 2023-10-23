#include "boot_screen.h"
#include "controller.h"
#include "drive_mode.h"
#include "sd_message.h"
#include "user_config.h"

BootScreen::BootScreen(): BaseScreen("", false) {

}

BaseScreen* BootScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  if (entered_at + 3000 < millis()) {
    if (controller.get_data().sd_card_status != SDInterface::SdStatus::e_sd_config_status_ok) {
      return SdMessageScreen::i();
    }
    return DriveModeScreen::i();
  }
  return nullptr;
}

void BootScreen::enter_screen(Controller& controller) {
  entered_at = millis();
}

void BootScreen::leave_screen(Controller& controller) {
}

void BootScreen::render(const worker_map_t& workers, const handler_map_t& handlers) {
  // Display something
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Lcd.drawString("bGeigie Zen", 90, 50, 4);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("Some splash screen device details.", 5, 100, 2);
  M5.Lcd.drawString(VERSION_STRING, 5, 120, 2);
  // Display safecast copyright
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("SAFECAST", 230, 215, 1);
  M5.Lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5.Lcd.drawString("2023", 285, 215, 1);
}
