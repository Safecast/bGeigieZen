

#include <menu.hpp>

void InitState::on_entry(MenuContext *context) {
  /*DEBUG*/ Serial.println("InitState::on_entry");
  M5.Lcd.clear();
  M5.lcd.setRotation(3);
  M5.Lcd.setTextDatum(BL_DATUM);  // By default, text x,y is bottom left corner
  // Draw the initial menu and set up the buttons.
  M5.Lcd.drawString("Menu InitState",10, 60, 2);
}

void InitState::on_exit(MenuContext *context) {/*DEBUG*/ Serial.println("InitState::on_exit");}
void InitState::update(MenuContext *context) {/*DEBUG*/ Serial.println("InitState::update");}
