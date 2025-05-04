#ifndef SCREENS_AIR_MODE_H
#define SCREENS_AIR_MODE_H

#include "base_screen.h"
#include "workers/gps_connector.h"

class AirModeScreen : public BaseScreen {
 public:
  explicit AirModeScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  bool _logging_available;
  bool _currently_logging;
  bool _transmitting_data;
  bool _wifi_enabled;
  bool _ble_enabled;
  UbxDynamicModel _previous_gps_model;
};

extern AirModeScreen AirModeScreen_i;

#endif //SCREENS_AIR_MODE_H
