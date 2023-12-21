#ifndef SCREENS_BOOT_SCREEN_H
#define SCREENS_BOOT_SCREEN_H

#include "base_screen.h"

class BootScreen : public BaseScreen {
 public:
  /**
   * Singleton
   */
  explicit BootScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:

  uint32_t _entered_at;
};

extern BootScreen BootScreen_i;

#endif //SCREENS_BOOT_SCREEN_H
