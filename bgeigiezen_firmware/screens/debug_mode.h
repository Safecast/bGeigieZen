#ifndef SCREENS_DEBUG_SCREEN_H
#define SCREENS_DEBUG_SCREEN_H

#include "base_screen.h"

class DebugModeScreen : public BaseScreen {
 public:
  static DebugModeScreen* i() {
    static DebugModeScreen screen;
    return &screen;
  }

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  explicit DebugModeScreen();
};

#endif //SCREENS_DEBUG_SCREEN_H
