#ifndef BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_MENU_ITEMS_H
#define BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_MENU_ITEMS_H

#include "base_screen.h"

#define MAIN_MENU_MAX 8

/**
 * Menu items is a separate screen render on top of the "current screen" in the gfx screen
 */
class MenuWindow : public BaseScreenWithMenu {
 public:

  explicit MenuWindow();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;

  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
};

extern MenuWindow MenuWindow_i;

#endif //BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_MENU_ITEMS_H
