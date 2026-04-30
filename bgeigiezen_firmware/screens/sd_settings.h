#ifndef SCREENS_SD_SETTINGS_H
#define SCREENS_SD_SETTINGS_H

#include "base_screen.h"

class SdSettingsScreen : public BaseScreenWithMenu {
 public:
  enum SdSettingsPage {
    e_sd_page_load_config,
    e_sd_page_save_config,
    e_sd_page_wipe,
    e_sd_page_back,
    e_sd_MENU_MAX,
  };

  explicit SdSettingsScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  void render_sd_wipe(const worker_map_t& workers, const handler_map_t& handlers);
};

extern SdSettingsScreen SdSettingsScreen_i;

#endif //SCREENS_SD_SETTINGS_H
