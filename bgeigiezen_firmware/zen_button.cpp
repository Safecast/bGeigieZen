#include "zen_button.h"
#include "debugger.h"


ZenButton::ZenButton(Button& m5_button) :
    Worker<bool>(),
    _m5_button(m5_button) {
}

ZenButton::~ZenButton() {
}

bool ZenButton::activate(bool retry) {
  return true;
}

int8_t ZenButton::produce_data() {
  _m5_button.read();
  if (_m5_button.wasPressed()) {
    return e_worker_data_read;
  }
  return e_worker_idle;
}
