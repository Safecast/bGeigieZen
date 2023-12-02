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

  static SdMessageScreen* i() {
    static SdMessageScreen screen;
    return &screen;
  }

  virtual BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  virtual void enter_screen(Controller& controller) override;
  virtual void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  explicit SdMessageScreen();

  SdMessageType error_type;
};

#endif //SCREENS_SD_ERROR_SCREEN_H
