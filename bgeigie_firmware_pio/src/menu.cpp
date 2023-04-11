
#include <M5Core2.h>

#include <display.hpp>
#include <menu.hpp>

DimBlankTiming::DimBlankTiming(uint32_t delay_before, const char *item_label) :
  delay{delay_before},
  label{item_label}
  {}


    void MenuContext::goto_state(MenuState *new_state) {
      if(this->current_state != nullptr) {
        /*DEBUG*/ Serial.println("goto_state: current_state != nullptr, executing on_exit()");
        this->current_state->on_exit(this);
      }
      this->current_state = new_state;
      /*DEBUG*/ Serial.println("goto_state: executing on_entry()");
      this->current_state->on_entry(this);
    }

    void MenuContext::update() {
      if(this->current_state != nullptr) {
        this->current_state->update(this);
      }
    }

void InitState::on_entry(MenuContext *context) {
  /*DEBUG*/ Serial.println("InitState::on_entry");
  M5.Lcd.clear();
  M5.lcd.setRotation(3);
  M5.Lcd.setTextDatum(BL_DATUM);  // By default, text x,y is bottom left corner

  /** TODO: Eliminate duplication from Display::draw_base and draw_navbar */
  // Display safecast copyright
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("SAFECAST", 230, 215, 1);
  M5.Lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5.Lcd.drawString("2023", 285, 215, 1);

  // Navbar: show only one button (N.B. hot zones are still live)
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  auto previous_datum = M5.Lcd.getTextDatum();
  M5.Lcd.setTextDatum(BC_DATUM);
  Button &b = M5.BtnB;
  M5.Lcd.drawString("RETURN", b.x+(b.w/2), b.y+(b.h-10), 2);
  M5.Lcd.setTextDatum(previous_datum);

  // Draw the initial menu and set up the buttons.
  M5.Lcd.drawString("Menu InitState",10, 20, 2);

  Button &dbbutton = context->button_dimblank;
  char dbbutton_label [15] = {'\0'};
  strncpy(dbbutton_label, "30s /  2m", sizeof(dbbutton_label)-1);
  // context->button_dimblank.setLabel("BLAH");
  context->button_dimblank.draw();
  M5.Lcd.setTextDatum(ML_DATUM);
  M5.Lcd.drawString("Blank / Dim", 10, dbbutton.y+(dbbutton.h/2), 4);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString(dbbutton_label,
                    dbbutton.x+(dbbutton.w/2),
                    dbbutton.y+(dbbutton.h/2), 4);
}

void InitState::on_exit(MenuContext *context) {/*DEBUG*/ Serial.println("InitState::on_exit");}
void InitState::update(MenuContext *context) {/*DEBUG*/ Serial.println("InitState::update");}
