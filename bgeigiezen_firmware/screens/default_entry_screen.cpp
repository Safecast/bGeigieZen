#include "default_entry_screen.h"
#include "drive_mode.h"
#include "fixed_mode.h"
<<<<<<< HEAD
#include "flight_mode.h"
=======
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
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
<<<<<<< HEAD
    case LocalStorage::e_operational_mode_flight:
      return &FlightModeScreen_i;
=======
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
    default:
      return &DriveModeScreen_i;
  }
}
