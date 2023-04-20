
#include <M5Core2.h>

#include <display.hpp>
#include <menu.hpp>

// InitState mstate{};
ConfigState cfgstate{};

DimBlankTiming::DimBlankTiming(uint32_t delay_dimming,
                                uint32_t delay_blanking,
                                const char *item_label) :
  delay_dimming{delay_dimming},
  delay_blanking{delay_blanking},
  label{item_label}
  {}

void MenuContext::goto_state(MenuState *new_state, Display *display ) {
  main_display = display;
  goto_state(new_state);
}

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
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("Menu InitState",10, 20, 2);

  Button &dbbutton = context->button_dimblank;
  Button &cfgbutton = context->button_config_network;
  
  // get the current values of dim/blank timing
  auto dimblankchoice = context->dimblank_choices[context->dimblank_idx];
  context->button_dimblank.draw();
  M5.Lcd.setTextDatum(ML_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("Blank / Dim", 10, dbbutton.y+(dbbutton.h/2), 4);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString(dimblankchoice.label,
                    dbbutton.x+(dbbutton.w/2),
                    dbbutton.y+(dbbutton.h/2), 4);
  context->button_config_network.draw();
  M5.Lcd.setTextDatum(ML_DATUM);
  M5.Lcd.drawString("WiFi Config", 10, cfgbutton.y+(cfgbutton.h/2), 4);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString("START",
                    cfgbutton.x+(cfgbutton.w/2),
                    cfgbutton.y+(cfgbutton.h/2), 4);
}

void InitState::on_exit(MenuContext *context) {/*DEBUG*/ Serial.println("InitState::on_exit");}

void InitState::update(MenuContext *context) {
  /*DEBUG*/ //Serial.println("InitState::update");

  // get the current values of dim/blank timing
  Button &dbbutton = context->button_dimblank;
  Button &cfgbutton = context->button_config_network;
if(cfgbutton.wasReleased()) {
  /*DEBUG*/ Serial.println("Config Button was pressed/released");
  context->goto_state(&cfgstate);
}
else if(dbbutton.wasReleased()) {
    // cycle through all choices
    context->dimblank_idx = context->dimblank_idx >= 4 ?
                            0 : context->dimblank_idx + 1;
    auto dimblankchoice = context->dimblank_choices[context->dimblank_idx];
    context->main_display->set_dimblank_delays(dimblankchoice.delay_dimming,
                                               dimblankchoice.delay_blanking);

    /*DEBUG*/ Serial.println(dimblankchoice.label);

    context->button_dimblank.draw();
    M5.Lcd.setTextDatum(ML_DATUM);
    M5.Lcd.drawString("Blank / Dim", 10, dbbutton.y+(dbbutton.h/2), 4);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString(dimblankchoice.label,
                      dbbutton.x+(dbbutton.w/2),
                      dbbutton.y+(dbbutton.h/2), 4);
  }
}

/* 
 * Configure the IP networking with captive access point
*/
void ConfigState::on_entry(MenuContext *context) {
  /*DEBUG*/ Serial.println("ConfigState::on_entry");
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
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("Menu ConfigState",10, 20, 2);

}
void ConfigState::on_exit(MenuContext *context) {/*DEBUG*/ Serial.println("ConfigState::on_exit");}

void ConfigState::update(MenuContext *context) {
  /*DEBUG*/ //Serial.println("ConfigState::update");
}
