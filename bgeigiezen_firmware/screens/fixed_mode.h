#ifndef SCREENS_FIXED_SCREEN_H
#define SCREENS_FIXED_SCREEN_H

#include "base_screen.h"

class FixedModeScreen : public BaseScreen {
 public:
  explicit FixedModeScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  uint32_t _total_posted;
  uint32_t _last_sent;
};

extern FixedModeScreen FixedModeScreen_i;

#endif //SCREENS_FIXED_SCREEN_H
