#ifndef SCREENS_CONFIG_SCREEN_H
#define SCREENS_CONFIG_SCREEN_H

#include "base_screen.h"


class ConfigModeScreen : public BaseScreenWithMenu {
 public:

  enum ConfigModePage {
    e_config_page_main,
    e_config_page_ap,
    e_config_page_wifi,
    e_config_page_load_sd_config,
    e_config_page_save_config_to_sd,
<<<<<<< HEAD
    e_config_page_sd_wipe,
    e_config_page_reset_dose,
    e_config_page_cpm_threshold,
    e_config_page_click_sound,
    e_config_page_error_alert_sound,
    e_config_page_audio_volume,
    e_config_page_dim_brightness,
    e_config_page_set_home_gps,
    e_config_page_reset_all,
    e_config_page_back_to_main,
=======
    e_config_page_reset,
    e_config_page_reset_all,
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
    e_config_MENU_MAX,
  };

  explicit ConfigModeScreen();

  BaseScreen* handle_input(Controller& controller, const worker_map_t& workers) override;
  void enter_screen(Controller& controller) override;
  void leave_screen(Controller& controller) override;

 protected:
  void render(const worker_map_t& workers, const handler_map_t& handlers, bool force) override;

 private:

  void render_page_main(const worker_map_t& workers, const handler_map_t& handlers);
  void render_page_ap(const worker_map_t& workers, const handler_map_t& handlers);
  void render_page_wifi(const worker_map_t& workers, const handler_map_t& handlers);
<<<<<<< HEAD
  void render_sd_wipe(const worker_map_t& workers, const handler_map_t& handlers);
  void render_reset_device_sd(const worker_map_t& workers, const handler_map_t& handlers);
  void render_cpm_threshold_page(const worker_map_t& workers, const handler_map_t& handlers);
  void render_click_sound_page(const worker_map_t& workers, const handler_map_t& handlers);
  void render_error_alert_sound_page(const worker_map_t& workers, const handler_map_t& handlers);
  void render_audio_volume_page(const worker_map_t& workers, const handler_map_t& handlers);
  void render_dim_brightness_page(const worker_map_t& workers, const handler_map_t& handlers);
  void render_set_home_gps_page(const worker_map_t& workers, const handler_map_t& handlers);
=======
  void render_reset_device(const worker_map_t& workers, const handler_map_t& handlers);
  void render_reset_device_sd(const worker_map_t& workers, const handler_map_t& handlers);
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416

  enum MainPageInfoSection {
    e_config_section_device,
    e_config_section_location,
    e_config_section_connection,
    e_config_section_MAX,
  };

  uint8_t _main_page_info_section;
};

extern ConfigModeScreen ConfigModeScreen_i;

#endif //SCREENS_CONFIG_SCREEN_H
