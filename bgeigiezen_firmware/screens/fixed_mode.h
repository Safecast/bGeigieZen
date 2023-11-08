#ifndef SCREENS_FIXED_SCREEN_H
#define SCREENS_FIXED_SCREEN_H

#include "base_screen.h"

class FixedModeScreen : public BaseScreen {
 public:
  static FixedModeScreen* i() {
    static FixedModeScreen screen;
    return &screen;
  }

  virtual BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  virtual void enter_screen(Controller& controller) override;
  virtual void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  explicit FixedModeScreen();
};

#endif //SCREENS_FIXED_SCREEN_H