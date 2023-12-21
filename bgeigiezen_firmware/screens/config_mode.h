#ifndef SCREENS_CONFIG_SCREEN_H
#define SCREENS_CONFIG_SCREEN_H

#include "base_screen.h"

class ConfigModeScreen : public BaseScreen {
 public:
  explicit ConfigModeScreen();

  virtual BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  virtual void enter_screen(Controller& controller) override;
  virtual void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
};

extern ConfigModeScreen ConfigModeScreen_i;

#endif //SCREENS_CONFIG_SCREEN_H
