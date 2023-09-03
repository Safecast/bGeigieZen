#ifndef BGEIGIEZEN_GFX_SCREEN_H_
#define BGEIGIEZEN_GFX_SCREEN_H_

#include <Arduino.h>
#include <screens/base_screen.h>
#include <Supervisor.hpp>
#include "screens/menu_window.h"

/**
 * M5 Screen renderer
 */
class GFXScreen : public Supervisor {
public:

  enum ScreenType {
    e_screen_splash,
    e_screen_init_no_sd_no_data, // Warning screen: must have sd card for initial setup
    e_screen_init_empty_sd_no_data, // Warning screen: must have SAFEZEN.txt for initial setup (could be invalid SAFEZEN.txt)
    e_screen_init_empty_sd_mem_data, // Press to init new sd card (write SAFEZEN.txt)
    e_screen_init_no_sd_mem_data, // Can continue in no-sd mode
    e_screen_init_zen_sd_no_data, // Press to start Zen (read from SAFEZEN.txt, show device id), is first-time startup (new user)
    e_screen_drive_mode,
    e_screen_survey_mode,
    e_screen_fixed_mode,
    e_screen_upload_drive,
    e_screen_config_mode,
  };

  explicit GFXScreen();
  virtual ~GFXScreen() = default;

  void handle_report(const worker_map_t &workers, const handler_map_t &handlers) override;
  void initialize() override;

private:
  void off();
  void clear();

  void setBrightness(uint8_t lvl, bool overdrive = false);

  unsigned long _last_render;
  BaseScreen* _screen;
  MenuWindow* _menu;
};

#endif //BGEIGIEZEN_GFX_SCREEN_H_
