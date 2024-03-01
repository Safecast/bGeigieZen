#include "config_mode.h"
#include "identifiers.h"
#include "menu_window.h"
#include "user_config.h"
#include "workers/local_storage.h"
#include "workers/zen_button.h"

ConfigModeScreen ConfigModeScreen_i;

ConfigModeScreen::ConfigModeScreen() : BaseScreen("Settings", true) {
}

BaseScreen* ConfigModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto reset_button = workers.worker<ZenButton>(k_worker_button_1);
  if (reset_button->is_fresh() && reset_button->get_data().shortPress) {
    M5.Lcd.clear();
    M5.Lcd.setTextColor(LCD_COLOR_ERROR, LCD_COLOR_BACKGROUND);
    M5.Lcd.setRotation(3);
    M5.Lcd.drawString("Resetting device...", 60, 133, 4);
    delay(500);
    controller.reset_all();
  }
  auto buttons_2 = workers.worker<ZenButton>(k_worker_button_2);
  if (buttons_2->is_fresh() && buttons_2->get_data().shortPress) {
    if (controller.load_sd_config()) {
      set_status_message(F(" LOADED SETTINGS FROM SAFEZEN.txt "));
    }
  }
  auto menu_button = workers.worker<ZenButton>(k_worker_button_3);
  if (menu_button->is_fresh() && menu_button->get_data().shortPress) {
    return &MenuWindow_i;
  }
  return nullptr;
}

void ConfigModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  drawButton1("Reset device");
  drawButton2("Load SD conf");
  drawButton3("Menu");


  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawString("Work in progress... for now please use SD card", 10, 46, 2);
  M5.Lcd.drawString("to change device settings!", 10, 62, 2);

  // Temp print out device settings on screen
  const auto& config = *workers.worker<LocalStorage>(k_worker_local_storage);
  M5.Lcd.setCursor(0, 78, 1);
  M5.Lcd.printf("device id:        %d      \n", config.get_device_id());
  M5.Lcd.printf("ap password:      %s      \n", config.get_ap_password());
  M5.Lcd.printf("alarm threshold:  %d      \n", config.get_alarm_threshold());
  M5.Lcd.printf("manual logging:   %s      \n", config.get_manual_logging() ? "Enabled" : "Disabled");
  M5.Lcd.printf("dim timeout:      %d seconds     \n", config.get_screen_dim_timeout());
  M5.Lcd.printf("off timeout:      %d seconds     \n", config.get_screen_off_timeout());
  M5.Lcd.printf("screensaver:      %s      \n", config.get_animated_screensaver() ? "Enabled" : "Disabled");
  M5.Lcd.printf("wifi ssid:        %s      \n", config.get_wifi_ssid());
  M5.Lcd.printf("wifi password:    %s      \n", config.get_wifi_password());
  M5.Lcd.printf("api key:          %s      \n", config.get_api_key());
  M5.Lcd.printf("fixed longitude:  %0.6f      \n", config.get_fixed_longitude());
  M5.Lcd.printf("fixed latitude:   %0.6f      \n", config.get_fixed_latitude());
}

void ConfigModeScreen::enter_screen(Controller& controller) {
}

void ConfigModeScreen::leave_screen(Controller& controller) {
}
