#ifndef SCREENS_DRIVE_SCREEN_H
#define SCREENS_DRIVE_SCREEN_H

#include "base_screen.h"

class DriveModeScreen : public BaseScreen {
 public:
  static DriveModeScreen* i() {
    static DriveModeScreen screen;
    return &screen;
  }

  BaseScreen* handle_input(const worker_map_t &workers) override;
  void render(TFT_eSprite& sprite, const worker_map_t &workers, const handler_map_t &handlers) override;
  void leave_screen() override;

 private:
  explicit DriveModeScreen();
};

#endif //SCREENS_BOOT_SCREEN_H
