#ifndef SCREENS_SOUND_SETTINGS_H
#define SCREENS_SOUND_SETTINGS_H

#include "base_screen.h"

class SoundSettingsScreen : public BaseScreenWithMenu {
 public:
  enum SoundSettingsPage {
    e_sound_page_audio,
    e_sound_page_cpm_threshold,
    e_sound_page_back,
    e_sound_MENU_MAX,
  };

  explicit SoundSettingsScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:
  void render_audio_page(const worker_map_t& workers, const handler_map_t& handlers);
  void render_cpm_threshold_page(const worker_map_t& workers, const handler_map_t& handlers);

  enum AudioField {
    e_audio_field_volume,
    e_audio_field_clicks,
    e_audio_field_alarm,
    e_audio_field_MAX,
  };
  uint8_t _audio_field;
};

extern SoundSettingsScreen SoundSettingsScreen_i;

#endif //SCREENS_SOUND_SETTINGS_H
