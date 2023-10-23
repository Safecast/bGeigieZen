#ifndef SCREENS_DRIVE_SCREEN_H
#define SCREENS_DRIVE_SCREEN_H

#include "base_screen.h"

class DriveModeScreen : public BaseScreen {
 public:
  static DriveModeScreen* i() {
    static DriveModeScreen screen;
    return &screen;
  }

  BaseScreen* handle_input(Controller& controller, const worker_map_t &workers) override;
  void render(const worker_map_t &workers, const handler_map_t &handlers) override;
  void leave_screen(Controller& controller) override;

 private:
  explicit DriveModeScreen();
};

#endif //SCREENS_DRIVE_SCREEN_H
