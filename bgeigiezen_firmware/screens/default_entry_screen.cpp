#include "default_entry_screen.h"
#include "drive_mode.h"
#include "fixed_mode.h"
#include "identifiers.h"
#include "satellite_view.h"
#include "survey_mode.h"

DefaultEntryScreen DefaultEntryScreen_i;

DefaultEntryScreen::DefaultEntryScreen() : BaseScreen("", false){
}

BaseScreen* DefaultEntryScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);

  switch (settings->get_last_mode()) {
    case LocalStorage::e_operational_mode_drive:
      return &DriveModeScreen_i;
    case LocalStorage::e_operational_mode_survey:
      return &SurveyModeScreen_i;
    case LocalStorage::e_operational_mode_fixed:
      return &FixedModeScreen_i;
    case LocalStorage::e_operational_mode_satellite:
      return &SatelliteViewScreen_i;
    default:
      return &DriveModeScreen_i;
  }
}
