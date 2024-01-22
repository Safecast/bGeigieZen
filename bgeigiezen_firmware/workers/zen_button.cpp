#include "zen_button.h"
#include "debugger.h"

#define SHORT_PRESS_DURATION 10
#define LONG_PRESS_DURATION 2400


ZenButton::ZenButton(Button& m5_button) :
    Worker<ButtonState>({false, false, false}),
    _m5_button(m5_button) {
}

ZenButton::~ZenButton() {
}

bool ZenButton::activate(bool retry) {
  return true;
}

int8_t ZenButton::produce_data() {
  data.currentlyPressed = _m5_button.isPressed();
  data.longPress = _m5_button.wasReleasefor(LONG_PRESS_DURATION);
  data.shortPress = _m5_button.wasReleasefor(SHORT_PRESS_DURATION) && !data.longPress;
  if (data.longPress || data.shortPress) {
//    DEBUG_PRINTLN("Button pressed");
    return e_worker_data_read;
  }
  return e_worker_idle;
}
