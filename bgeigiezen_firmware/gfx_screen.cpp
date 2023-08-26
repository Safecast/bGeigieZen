#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

#include "gfx_screen.h"
#include "identifiers.h"
#include "controller.h"
#include "debugger.h"
#include "screens/boot_screen.h"

static constexpr uint8_t LEVEL_BRIGHT = 35;  // max brightness = 36
static constexpr uint8_t LEVEL_DIMMED = 10;
static constexpr uint8_t LEVEL_BLANKED = 0;
static constexpr uint32_t DELAY_DIMMING_DEFAULT = 2 * 60 * 1000;  // ms before dimming screen
static constexpr uint32_t DELAY_BLANKING_DEFAULT = 3 * 60 * 1000;  // ms before blanking screen


GFXScreen::GFXScreen() : Supervisor(), _last_render(0), _screen(nullptr), _menu(MenuWindow::i()), _sprite(&M5.Lcd) {
}

void GFXScreen::initialize() {
#ifdef M5_CORE2
  // Change touch screen buttons slightly to overlap with button indicators
  M5.BtnA.set(8, 230, 90, 50);
  M5.BtnB.set(115, 230, 90, 50);
  M5.BtnC.set(222, 230, 90, 50);
#elif M5_BASIC
#endif
  _sprite.createSprite(300, 180);
//  _sprite.setSwapBytes(false);
  _screen = BootScreen::i();
  clear();
}

void GFXScreen::off() {
  // Clear the graphics screen
  M5.Lcd.clear();
  setBrightness(LEVEL_BLANKED);
}

//setup brightness by Rob Oudendijk 2023-03-13
void GFXScreen::setBrightness(uint8_t lvl, bool overdrive) {
#ifdef M5_CORE2
  // The backlight brightness is in steps of 25 in AXP192.cpp
  // calculation in SetDCVoltage: ( (voltage - 700) / 25 )
  // 2325 is the minimum "I can just about see a glow in a dark room" level of brightness.
  // 2800 is the value set by the AXP library as "standard" bright backlight.
  int v = lvl * 25 + 2300;

  // Clamp to safe values.
  if (v < 2300) v = 2300;
  if (overdrive) {
    if (v > 3200) v = 3200; // maximum of 3.2 volts, 3200 (uint8_t lvl  = 36) absolute max!
  }
  else {
    if (v > 2800) v = 2800; // maximum of 2.8 volts, 2800 (uint8_t lvl  = 20)
  }

  // Minimum brightness means turn off the LCD backlight.
  if (v == 2300) {
    // LED set to minimum brightness? Turn off.

    M5.Axp.SetDCDC3(false);
    return;
  }
  else {
    // Ensure backlight is on. (magic name = DCDC3)
    M5.Axp.SetDCDC3(true);
  }

  // Set the LCD backlight voltage. (magic number = 2)

  M5.Axp.SetDCVoltage(2, v);

#elif M5_BASIC
  // M5Stack Basic uses LCD Brightness (0: Off - 255:Full)
  int v = lvl * 25;
  M5.Lcd.setBrightness(v);
#endif
}

void GFXScreen::clear() {
  // Clear display
  M5.Lcd.clear();
  _sprite.setTextDatum(BL_DATUM);  // By default, text x,y is bottom left corner
  setBrightness(LEVEL_BRIGHT);
}

void GFXScreen::handle_report(const worker_map_t& workers, const handler_map_t& handlers) {
  if ((!workers.any_updates() && !handlers.any_updates())) {
    return;
  }
  if (_screen) {

    BaseScreen* new_screen;
    if (_menu->is_open()) {
      new_screen = _menu->handle_input(workers);
      if (!new_screen && !_menu->is_open()) {
        // Closed menu
        clear();
      }
    } else {
      new_screen = _screen->handle_input(workers);
      if (new_screen == _menu) {
        // opened menu, not a new screen
        _menu->enter_screen();
        new_screen = nullptr;
      }
    }
    if (new_screen) {
      _screen->leave_screen();
      _menu->leave_screen();
      new_screen->enter_screen();
      _screen = new_screen;
      clear();
    }

    _sprite.fillRect(0, 0, 320, 240, BLACK);
    _screen->render(_sprite, workers, handlers);
    _menu->render(_sprite, workers, handlers);
    M5.Lcd.setRotation(3);
    _sprite.pushSprite(0, 0);
    DEBUG_PRINTLN("Sprite pushed");
    M5.Lcd.setRotation(1);
  }

  _last_render = millis();
}
