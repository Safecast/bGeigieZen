#ifndef SCREENS_FIXED_SCREEN_H
#define SCREENS_FIXED_SCREEN_H

#include "base_screen.h"

class FixedModeScreen : public BaseScreen {
 public:
  static FixedModeScreen* i() {
    static FixedModeScreen screen;
    return &screen;
  }

  virtual BaseScreen* handle_input(const worker_map_t &workers) override;
  virtual void render(const worker_map_t &workers, const handler_map_t &handlers) override;
  virtual void enter_screen() override;
  virtual void leave_screen() override;

 private:
  explicit FixedModeScreen();
};

#endif //SCREENS_FIXED_SCREEN_H
