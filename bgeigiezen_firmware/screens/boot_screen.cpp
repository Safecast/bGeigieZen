#include "boot_screen.h"
#include "drive_mode.h"
#include "user_config.h"

BootScreen::BootScreen() {

}

BaseScreen* BootScreen::handle_input(const worker_map_t& workers) {
  if (entered_at + 1000 < millis()) {
    return DriveModeScreen::i();
  }
  return nullptr;
}

void BootScreen::enter_screen() {
  entered_at = millis();
}

void BootScreen::leave_screen() {
}

void BootScreen::render(TFT_eSprite& sprite, const worker_map_t& workers, const handler_map_t& handlers) {
  // Display something
  sprite.setCursor(10, 10);
  sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
  sprite.drawString("bGeigie Zen", 90, 50, 4);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  sprite.drawString("Some splash screen device details.", 5, 100, 2);
  sprite.drawString(VERSION_STRING, 5, 120, 2);
  // Display safecast copyright
  sprite.setTextFont(1);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  sprite.drawString("SAFECAST", 230, 215, 1);
  sprite.setTextColor(TFT_ORANGE, TFT_BLACK);
  sprite.drawString("2023", 285, 215, 1);
}
