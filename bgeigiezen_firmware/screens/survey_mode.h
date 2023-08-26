#ifndef SCREENS_SURVEY_SCREEN_H
#define SCREENS_SURVEY_SCREEN_H

#include "base_screen.h"

class SurveyModeScreen : public BaseScreen {
 public:
  static SurveyModeScreen* i() {
    static SurveyModeScreen screen;
    return &screen;
  }

  virtual BaseScreen* handle_input(const worker_map_t &workers) override;
  virtual void render(TFT_eSprite& sprite, const worker_map_t &workers, const handler_map_t &handlers) override;
  virtual void enter_screen() override;
  virtual void leave_screen() override;

 private:
  explicit SurveyModeScreen();
};

#endif //SCREENS_BOOT_SCREEN_H
