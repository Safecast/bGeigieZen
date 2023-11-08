#ifndef FIRST_TIME_STARTUP_SCREEN_H
#define FIRST_TIME_STARTUP_SCREEN_H

#include "base_screen.h"

class FirstTimeStartupScreen : public BaseScreen {
 public:
  static FirstTimeStartupScreen* i() {
    static FirstTimeStartupScreen screen;
    return &screen;
  }

  virtual BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  virtual void enter_screen(Controller& controller) override;
  virtual void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  explicit FirstTimeStartupScreen();
};

#endif //FIRST_TIME_STARTUP_SCREEN_H
