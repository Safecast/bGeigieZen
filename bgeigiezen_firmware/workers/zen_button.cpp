#include "zen_button.h"
#include "debugger.h"

#ifdef M5_GREY
#define SHORT_PRESS_DURATION 100
#else
#define SHORT_PRESS_DURATION 10
#endif
#define LONG_PRESS_DURATION 2400

ZenButton::ZenButton(Button& m5_button) : Worker<ButtonState>({false, false, false}),
                                          _m5_button(m5_button) {
}

ZenButton::~ZenButton() {
}

bool ZenButton::activate(bool retry) {
  return true;
}

int8_t ZenButton::produce_data() {
  data.longPress = data.currentlyPressed && _m5_button.wasReleasefor(LONG_PRESS_DURATION);
  data.shortPress = data.currentlyPressed && _m5_button.wasReleasefor(SHORT_PRESS_DURATION) && !data.longPress;
  data.currentlyPressed = _m5_button.isPressed();

  if (data.longPress || data.shortPress) {
    DEBUG_PRINTLN("Button pressed");
    return e_worker_data_read;
  }
  return e_worker_idle;
}
