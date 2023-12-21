#ifndef FIRST_TIME_STARTUP_SCREEN_H
#define FIRST_TIME_STARTUP_SCREEN_H

#include "base_screen.h"
#include "debugger.h"

class FirstTimeStartupScreen : public BaseScreen {
 public:
  explicit FirstTimeStartupScreen();

  virtual BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  virtual void enter_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
};

extern FirstTimeStartupScreen FirstTimeStartupScreen_i;

#endif //FIRST_TIME_STARTUP_SCREEN_H
