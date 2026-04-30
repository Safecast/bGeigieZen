#ifndef SCREENS_WIFI_SETTINGS_H
#define SCREENS_WIFI_SETTINGS_H

#include "base_screen.h"

class WifiSettingsScreen : public BaseScreenWithMenu {
 public:
  enum WifiSettingsPage {
    e_wifi_page_ap,
    e_wifi_page_local,
    e_wifi_page_back,
    e_wifi_MENU_MAX,
  };

  explicit WifiSettingsScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  void render_page_ap(const worker_map_t& workers, const handler_map_t& handlers);
  void render_page_local(const worker_map_t& workers, const handler_map_t& handlers);
};

extern WifiSettingsScreen WifiSettingsScreen_i;

#endif //SCREENS_WIFI_SETTINGS_H
