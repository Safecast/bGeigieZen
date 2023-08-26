#include "base_screen.h"
#include "user_config.h"

#define BUTTON_TEXT_LENGTH 12

void BaseScreen::drawButton(TFT_eSprite& sprite, uint16_t x, const char* text, uint32_t border_color) {
  if (strlen(text) > BUTTON_TEXT_LENGTH) {
    // Dont render long button text :(
    return;
  }

  sprite.fillRoundRect(x, -5, 90, 20, 4, TFT_BLACK);
  sprite.drawRoundRect(x, -5, 90, 20, 4, border_color);
  sprite.setTextColor(TFT_ORANGE, TFT_BLACK);
  sprite.drawString(text, (x + 45) - static_cast<uint16_t>(strlen(text) * 3), 10);
  sprite.flush();
}

void BaseScreen::drawButton1(TFT_eSprite& sprite, const char* text, uint32_t border_color) {
#ifdef M5_CORE2
  drawButton(sprite, 8, text, border_color);
#elif M5_BASIC
  drawButton(sprite, 20, text, border_color);
#endif
}

void BaseScreen::drawButton2(TFT_eSprite& sprite, const char* text, uint32_t border_color) {
#ifdef M5_CORE2
  drawButton(sprite, 115, text, border_color);
#elif M5_BASIC
  drawButton(sprite, 115, text, border_color);
#endif
}

void BaseScreen::drawButton3(TFT_eSprite& sprite, const char* text, uint32_t border_color) {
#ifdef M5_CORE2
  drawButton(sprite, 222, text, border_color);
#elif M5_BASIC
  drawButton(sprite, 210, text, border_color);
#endif
}

void BaseScreen::enter_screen() {
}

void BaseScreen::leave_screen() {
}
