#ifndef SCREENS_SURVEY_SCREEN_H
#define SCREENS_SURVEY_SCREEN_H

#include "base_screen.h"

class SurveyModeScreen : public BaseScreen {
 public:
  explicit SurveyModeScreen();

  virtual BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  virtual void enter_screen(Controller& controller) override;
  virtual void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:

  bool _logging_available;
  bool _currently_logging;
};

extern SurveyModeScreen SurveyModeScreen_i;

#endif //SCREENS_SURVEY_SCREEN_H
