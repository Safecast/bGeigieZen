#ifndef SCREENS_SURVEY_SCREEN_H
#define SCREENS_SURVEY_SCREEN_H

#include "base_screen.h"
#include "workers/gps_connector.h"

class SurveyModeScreen : public BaseScreen {
 public:
  explicit SurveyModeScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:

  bool _logging_available;
  bool _currently_logging;
  UbxDynamicModel _previous_gps_model;
  bool _gps_model_set;
};

extern SurveyModeScreen SurveyModeScreen_i;

#endif //SCREENS_SURVEY_SCREEN_H
