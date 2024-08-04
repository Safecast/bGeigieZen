#ifndef SCREENS_SATELLITE_VIEW_H
#define SCREENS_SATELLITE_VIEW_H

#include "base_screen.h"

class SatelliteViewScreen : public BaseScreenWithMenu {
 public:

  enum SatellitePage {
    e_satellite_page_main,
    e_satellite_page_set_default_config,
    e_satellite_page_set_na_config,
    e_satellite_page_set_eu_config,
    e_satellite_page_set_asia_config,
    e_satellite_page_cold_start,
    e_satellite_page_factory_reset,
    e_satellite_MENU_MAX,
  };

  explicit SatelliteViewScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 public:
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 private:
};

extern SatelliteViewScreen SatelliteViewScreen_i;

#endif //SCREENS_SATELLITE_VIEW_H
