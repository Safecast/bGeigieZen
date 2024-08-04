#ifndef BGEIGIEZEN_BUTTON_H_
#define BGEIGIEZEN_BUTTON_H_

#include <M5Unified.hpp>

#include <Arduino.h>
#include <Worker.hpp>

struct ButtonState {
  bool currentlyPressed;
  bool shortPress;
  bool longPress;
};

/**
 * M5 button wrapper as worker
 */
class ZenButton : public Worker<ButtonState> {
 public:
  explicit ZenButton(m5::Button_Class& m5_button);

  virtual ~ZenButton();

  int8_t produce_data() override;

 private:
  m5::Button_Class& _m5_button;
};

#endif //BGEIGIEZEN_BUTTON_H_
