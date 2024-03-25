#ifndef SCREENS_SATELLITE_VIEW_H
#define SCREENS_SATELLITE_VIEW_H

#include "base_screen.h"

class SatelliteViewScreen : public BaseScreen {
 public:
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
