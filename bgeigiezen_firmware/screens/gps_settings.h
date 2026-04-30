#ifndef SCREENS_GPS_SETTINGS_H
#define SCREENS_GPS_SETTINGS_H

#include "base_screen.h"

class GpsSettingsScreen : public BaseScreenWithMenu {
 public:
  enum GpsSettingsPage {
    e_gps_page_set_home,
    e_gps_page_back,
    e_gps_MENU_MAX,
  };

  explicit GpsSettingsScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  void render_set_home_gps_page(const worker_map_t& workers, const handler_map_t& handlers);
};

extern GpsSettingsScreen GpsSettingsScreen_i;

#endif //SCREENS_GPS_SETTINGS_H
