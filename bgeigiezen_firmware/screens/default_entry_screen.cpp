#include "default_entry_screen.h"
#include "drive_mode.h"
#include "identifiers.h"

DefaultEntryScreen DefaultEntryScreen_i;

DefaultEntryScreen::DefaultEntryScreen() : BaseScreen("", false){
}

BaseScreen* DefaultEntryScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  const auto& settings = workers.worker<LocalStorage>(k_worker_local_storage);

  // TODO: use settings to determine the next screen, but now drive mode by default
  return &DriveModeScreen_i;
}
