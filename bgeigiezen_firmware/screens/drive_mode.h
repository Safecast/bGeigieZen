#ifndef SCREENS_DRIVE_SCREEN_H
#define SCREENS_DRIVE_SCREEN_H

#include "base_screen.h"

class DriveModeScreen : public BaseScreen {
 public:
  explicit DriveModeScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void leave_screen(Controller& controller) override;

  const __FlashStringHelper* get_status_message(const worker_map_t& workers, const handler_map_t& handlers) const override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:

  bool _logging_available;
  bool _currently_logging;
  uint32_t _logging_start;
  uint32_t _logging_stop;
  double _distance_start;
};

extern DriveModeScreen DriveModeScreen_i;

#endif //SCREENS_DRIVE_SCREEN_H
