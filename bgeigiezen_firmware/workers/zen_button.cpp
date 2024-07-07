#include "zen_button.h"
#include "debugger.h"

#define SHORT_PRESS_DURATION 10
#define LONG_PRESS_DURATION 2400

ZenButton::ZenButton(Button& m5_button) : Worker<ButtonState>({false, false, false}),
                                          _m5_button(m5_button) {
}

ZenButton::~ZenButton() {
}

int8_t ZenButton::produce_data() {
  data.longPress = data.currentlyPressed && _m5_button.wasReleasefor(LONG_PRESS_DURATION);
  data.shortPress = data.currentlyPressed && _m5_button.wasReleasefor(SHORT_PRESS_DURATION) && !data.longPress;
  data.currentlyPressed = _m5_button.pressedFor(SHORT_PRESS_DURATION);
  if (data.longPress || data.shortPress) {
    ZEN_LOGD("Button pressed\n");
    return e_worker_data_read;
  }
  return e_worker_idle;
}
