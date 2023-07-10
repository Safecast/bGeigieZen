#include "zen_button.h"
#include "debugger.h"

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
  data.longPress = _m5_button.wasReleasefor(0) && _m5_button.wasReleasefor(2400);
  data.shortPress = _m5_button.wasReleasefor(0) && !data.longPress;
  return data.currentlyPressed || data.longPress || data.shortPress ? e_worker_data_read : e_worker_idle;
}
