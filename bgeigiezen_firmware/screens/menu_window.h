#ifndef BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_MENU_ITEMS_H
#define BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_MENU_ITEMS_H

#include "base_screen.h"

/**
 * Menu items is a separate screen render on top of the "current screen" in the gfx screen
 */
class MenuWindow : public BaseScreen {
 public:
  struct MenuItem {
    const char* title;
    const char* tooltip_l1;
    const char* tooltip_l2;
    const bool enabled;
  };

  static MenuWindow* i() {
    static MenuWindow screen;
    return &screen;
  }

  void render(const worker_map_t& workers, const handler_map_t& handlers) override;

  BaseScreen* handle_input(Controller& controller, const worker_map_t &workers) override;

  bool is_open() const;

 private:
  explicit MenuWindow();
 public:
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 private:
  bool menu_open;
  uint8_t menu_index;
  uint8_t max_index; // Set on enter menu
  MenuItem advanced_menu[5];
};

#endif //BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_MENU_ITEMS_H
