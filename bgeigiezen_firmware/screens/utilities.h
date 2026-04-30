#ifndef SCREENS_UTILITIES_H
#define SCREENS_UTILITIES_H

#include "base_screen.h"

class UtilitiesScreen : public BaseScreenWithMenu {
 public:
  enum UtilitiesPage {
    e_utilities_page_debug,            // transitions to DebugModeScreen
    e_utilities_page_factory_reset,
    e_utilities_page_reset_dose,
    e_utilities_page_info,
    e_utilities_page_back,
    e_utilities_MENU_MAX,
  };

  explicit UtilitiesScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  enum InfoSection {
    e_info_section_device,
    e_info_section_location,
    e_info_section_connection,
    e_info_section_MAX,
  };

  void render_info_page(const worker_map_t& workers, const handler_map_t& handlers);
  void render_factory_reset_page(const worker_map_t& workers, const handler_map_t& handlers);

  uint8_t _info_section;
};

extern UtilitiesScreen UtilitiesScreen_i;

#endif //SCREENS_UTILITIES_H
