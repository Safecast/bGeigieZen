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
  enum ConfigModePage {
    e_config_page_main,
    e_config_page_ap,
    e_config_page_wifi,
    e_config_page_load_sd_config,
    e_config_page_save_config_to_sd,
    e_config_page_reset,
    e_config_page_MAX,
  };

  void render_options_menu();
  void render_page_main(const worker_map_t& workers, const handler_map_t& handlers);
  void render_page_ap(const worker_map_t& workers, const handler_map_t& handlers);
  void render_page_wifi(const worker_map_t& workers, const handler_map_t& handlers);

  bool _options_menu;
  uint8_t _menu_index;
  ConfigModePage _page;
};

extern ConfigModeScreen ConfigModeScreen_i;

#endif //SCREENS_CONFIG_SCREEN_H
