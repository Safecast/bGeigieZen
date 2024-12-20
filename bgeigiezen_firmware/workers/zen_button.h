#ifndef BGEIGIEZEN_BUTTON_H_
#define BGEIGIEZEN_BUTTON_H_

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

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
  explicit ZenButton(Button& m5_button);

  virtual ~ZenButton();

  int8_t produce_data() override;

 private:
  Button& _m5_button;
};

#endif //BGEIGIEZEN_BUTTON_H_
