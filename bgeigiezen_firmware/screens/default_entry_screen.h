#ifndef SCREENS_DEFAULT_ENTRY_SCREEN_H
#define SCREENS_DEFAULT_ENTRY_SCREEN_H

#include "base_screen.h"

class DefaultEntryScreen : public BaseScreen {
 public:
  explicit DefaultEntryScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;

 private:

};

extern DefaultEntryScreen DefaultEntryScreen_i;

#endif //SCREENS_DEFAULT_ENTRY_SCREEN_H
