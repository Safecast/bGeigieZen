#ifndef SCREENS_DEFAULT_ENTRY_SCREEN_H
#define SCREENS_DEFAULT_ENTRY_SCREEN_H

#include "base_screen.h"

class DefaultEntryScreen : public BaseScreen {
 public:
  static DefaultEntryScreen* i() {
    static DefaultEntryScreen screen;
    return &screen;
  }

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;

 private:
  explicit DefaultEntryScreen();

};

#endif //SCREENS_DEFAULT_ENTRY_SCREEN_H
