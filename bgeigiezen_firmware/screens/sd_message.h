#ifndef SCREENS_SD_ERROR_SCREEN_H
#define SCREENS_SD_ERROR_SCREEN_H

#include "base_screen.h"

class SdMessageScreen : public BaseScreen {
 public:
  enum SdMessageType {
    k_unknown,
    k_no_sd_with_storage,
    k_no_sd_no_storage,
    k_empty_sd_with_storage,
    k_empty_sd_no_storage,
    k_config_sd_different_id,
    k_new_device,
  };

  explicit SdMessageScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:

  SdMessageType error_type;
};

extern SdMessageScreen SdMessageScreen_i;

#endif //SCREENS_SD_ERROR_SCREEN_H
