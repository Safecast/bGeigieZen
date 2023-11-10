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


GFXScreen::GFXScreen(LocalStorage& settings, Controller& controller) : Supervisor(), _controller(controller), _settings(settings), _last_render(0), _screen(nullptr), _menu(MenuWindow::i()) {
}

void GFXScreen::initialize() {
#ifdef M5_CORE2
  // Change touch screen buttons slightly to overlap with button indicators
  M5.BtnA.set(8, 230, 90, 50);
  M5.BtnB.set(115, 230, 90, 50);
  M5.BtnC.set(222, 230, 90, 50);
#else
#endif

  _screen = BootScreen::i();
  _screen->enter_screen(_controller);
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
  M5.Lcd.setTextDatum(BL_DATUM);  // By default, text x,y is bottom left corner
  M5.Lcd.setTextFont(1);
  if (_screen) {
    _screen->force_next_render();
  }
  setBrightness(LEVEL_BRIGHT);
}

void GFXScreen::handle_report(const worker_map_t& workers, const handler_map_t& handlers) {
  if (_screen) {

    BaseScreen* new_screen = nullptr;
    if (workers.any_updates()) {
      if (_menu->is_open()) {
        new_screen = _menu->handle_input(_controller, workers);
        if (new_screen || !_menu->is_open()) {
          // Closed menu
          DEBUG_PRINTLN("Menu closed");
          _menu->leave_screen(_controller);
          clear();
        }
      } else {
        new_screen = _screen->handle_input(_controller, workers);
        if (new_screen == _menu) {
          // opened menu, not a new screen
          DEBUG_PRINTLN("Menu opened");
          _menu->enter_screen(_controller);
          new_screen = nullptr;
        }
      }
      if (new_screen && new_screen != _screen) {
        DEBUG_PRINTF("New screen entered: %s\n", new_screen->get_title());
        _screen->leave_screen(_controller);
        _screen = new_screen;
        clear();
        _screen->enter_screen(_controller);
      }
    }

    if (workers.any_updates() || handlers.any_updates() || (millis() - _last_render > 1000)) {
      M5.Lcd.setRotation(3);
      if (_menu->is_open()) {
        _menu->do_render(workers, handlers);
      } else {
        _screen->do_render(workers, handlers);
      }

      if (_screen->has_status_bar()) {
        // Render bottom status bar
        M5.Lcd.drawLine(0, 220, 320, 220, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.drawString("(TODO: status icons)", 5, 233);
        M5.Lcd.drawString(_screen->get_title(), 315 - static_cast<uint8_t>((strlen(_screen->get_title()) * 6)), 233);
      }

      M5.Lcd.setRotation(1);
      _last_render = millis();

    }
  }



}
