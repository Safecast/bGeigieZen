#ifndef BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_MENU_ITEMS_H
#define BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_MENU_ITEMS_H

#include "base_screen.h"

#define ADVANCED_MENU_ITEMS 8

/**
 * Menu items is a separate screen render on top of the "current screen" in the gfx screen
 */
class MenuWindow : public BaseScreen {
 public:
  struct MenuItem {
    const char* title;
    const char* tooltip;
    bool enabled;
    BaseScreen* screen;
  };

  explicit MenuWindow();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;

  bool is_open() const;

  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:

  bool menu_open;
  uint8_t menu_index;
  MenuItem advanced_menu[ADVANCED_MENU_ITEMS];
};

extern MenuWindow MenuWindow_i;

#endif //BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_MENU_ITEMS_H
