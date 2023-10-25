#include "base_screen.h"
#include "user_config.h"

#define BUTTON_TEXT_MAX_LENGTH 13


BaseScreen::BaseScreen(const char* title, bool status_bar): _status_bar(status_bar) {
  if (strlen(title) < 20) {
    strcpy(_title, title);
  }
}

void BaseScreen::drawButton(uint16_t x, const char* text, ButtonState state) {
  if (strlen(text) > BUTTON_TEXT_MAX_LENGTH) {
    // Dont render long button text :(
    return;
  }

  int border_color = LCD_COLOR_BACKGROUND;
  int text_color = LCD_COLOR_ACTIVE;

  if (strlen(text)) {
    switch (state) {
      case e_button_default:
        border_color = LCD_COLOR_DEFAULT;
        break;
      case e_button_active:
        border_color = LCD_COLOR_ACTIVE;
        break;
      case e_button_disabled:
        border_color = LCD_COLOR_INACTIVE;
        text_color = LCD_COLOR_INACTIVE;
        break;
    }
  }

  M5.Lcd.drawRoundRect(x, -5, 90, 20, 4, border_color);
  M5.Lcd.setTextColor(text_color, LCD_COLOR_BACKGROUND);

  // Center the button text, making sure around the text is whitespace to clear previous texts
  char blob[BUTTON_TEXT_MAX_LENGTH + 1] = "             ";
  for (size_t i = 0; i < strlen(text); ++i) {
    blob[i + ((BUTTON_TEXT_MAX_LENGTH - strlen(text)) / 2)] = text[i];
  }
  if (strlen(text) % 2 != BUTTON_TEXT_MAX_LENGTH % 2) {
    blob[BUTTON_TEXT_MAX_LENGTH - 1] = '\0';
  }
  M5.Lcd.drawString(blob, (x + 45) - static_cast<uint16_t>(strlen(blob) * 3), 10);
  M5.Lcd.flush();
}

void BaseScreen::drawButton1(const char* text, ButtonState state) {
#ifdef M5_CORE2
  drawButton(8, text, state);
#elif M5_BASIC
  drawButton(20, text, state);
#endif
}

void BaseScreen::drawButton2(const char* text, ButtonState state) {
#ifdef M5_CORE2
  drawButton(115, text, state);
#elif M5_BASIC
  drawButton(115, text, state);
#endif
}

void BaseScreen::drawButton3(const char* text, ButtonState state) {
#ifdef M5_CORE2
  drawButton(222, text, state);
#elif M5_BASIC
  drawButton(210, text, state);
#endif
}

void BaseScreen::enter_screen(Controller& controller) {
}

void BaseScreen::leave_screen(Controller& controller) {
}

const char* BaseScreen::get_title() const {
  return _title;
}

bool BaseScreen::has_status_bar() const {
  return _status_bar;
}
