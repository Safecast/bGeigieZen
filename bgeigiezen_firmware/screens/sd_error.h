#ifndef SCREENS_SD_ERROR_SCREEN_H
#define SCREENS_SD_ERROR_SCREEN_H

#include "base_screen.h"

class SdErrorScreen : public BaseScreen {
 public:
  enum SdErrorType {
    k_unknown,
    k_no_sd_with_storage,
    k_no_sd_no_storage,
    k_empty_sd_with_storage,
    k_empty_sd_no_storage,
  };

  static SdErrorScreen* i() {
    static SdErrorScreen screen;
    return &screen;
  }

  virtual BaseScreen* handle_input(const worker_map_t &workers) override;
  virtual void render(const worker_map_t &workers, const handler_map_t &handlers) override;
  virtual void enter_screen() override;
  virtual void leave_screen() override;

 private:
  explicit SdErrorScreen();

  SdErrorType error_type;
};

#endif //SCREENS_SD_ERROR_SCREEN_H
