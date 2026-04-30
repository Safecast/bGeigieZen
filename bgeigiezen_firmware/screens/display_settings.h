#ifndef SCREENS_DISPLAY_SETTINGS_H
#define SCREENS_DISPLAY_SETTINGS_H

#include "base_screen.h"

class DisplaySettingsScreen : public BaseScreenWithMenu {
 public:
  enum DisplaySettingsPage {
    e_display_page_dim_brightness,
    e_display_page_back,
    e_display_MENU_MAX,
  };

  explicit DisplaySettingsScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  void render_dim_brightness_page(const worker_map_t& workers, const handler_map_t& handlers);
};

extern DisplaySettingsScreen DisplaySettingsScreen_i;

#endif //SCREENS_DISPLAY_SETTINGS_H
