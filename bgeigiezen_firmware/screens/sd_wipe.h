#ifndef BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_SD_WIPE_H
#define BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_SD_WIPE_H

#include "base_screen.h"

/**
 * SD Wipe screen for wiping all data from the SD card
 */
class SDWipeScreen : public BaseScreen {
 public:
  explicit SDWipeScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  enum WipeState {
    CONFIRM,    // Ask for confirmation
    WIPING,     // Wiping in progress
    COMPLETE    // Wipe complete
  };

  WipeState _state;
  bool _wipe_success;
};

extern SDWipeScreen SDWipeScreen_i;

#endif //BGEIGIEZEN_BGEIGIEZEN_FIRMWARE_SCREENS_SD_WIPE_H
