#ifndef SCREENS_CONFIG_SCREEN_H
#define SCREENS_CONFIG_SCREEN_H

#include "base_screen.h"

/**
 * Top-level Settings router.
 * Each menu entry transitions to a dedicated submenu screen
 * (Display / GPS / Sound / SD card / WiFi / Utilities) or back
 * to the main menu.
 */
class ConfigModeScreen : public BaseScreenWithMenu {
 public:
  enum ConfigModePage {
    e_config_page_display,
    e_config_page_gps,
    e_config_page_sound,
    e_config_page_sd,
    e_config_page_wifi,
    e_config_page_utilities,
    e_config_page_back_to_main,
    e_config_MENU_MAX,
  };

  explicit ConfigModeScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;
};

extern ConfigModeScreen ConfigModeScreen_i;

#endif //SCREENS_CONFIG_SCREEN_H
