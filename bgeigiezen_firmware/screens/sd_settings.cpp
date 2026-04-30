#include "sd_settings.h"
#include "config_mode.h"
#include "controller.h"
#include "identifiers.h"
#include "user_config.h"
#include "utils/error_beep.h"
#include "utils/sd_wrapper.h"
#include "workers/zen_button.h"

SdSettingsScreen SdSettingsScreen_i;

static const SdSettingsScreen::MenuItem SD_MENU[SdSettingsScreen::e_sd_MENU_MAX] = {
    {.title="Load from SD", .tooltip="Read settings file from the SD-card and set to device", .enabled=true, .screen=nullptr},
    {.title="Save to SD", .tooltip="Write current device settings to the SD-card config", .enabled=true, .screen=nullptr},
    {.title="Wipe SD card", .tooltip="Delete all log files from the SD card", .enabled=true, .screen=nullptr},
    {.title="Back", .tooltip="Return to Settings", .enabled=true, .screen=&ConfigModeScreen_i},
};

SdSettingsScreen::SdSettingsScreen() : BaseScreenWithMenu("SD card", true) {
  _current_page = e_sd_MENU_MAX;  // sentinel: "show menu, no action page"
}

void SdSettingsScreen::enter_screen(Controller& controller) {
  switch (_current_page) {
    case e_sd_page_load_config:
      if (controller.load_sd_config()) {
        set_status_message(F(" SD CONFIG LOADED, settings have been updated! "));
      } else {
        set_status_message(F(" LOAD CONFIG FAILED, no config or SD-card! "));
      }
      // Action complete — bounce back to the submenu
      _current_page = e_sd_MENU_MAX;
      _menu_index = e_sd_page_load_config;
      open_menu(true);
      break;
    case e_sd_page_save_config:
      if (controller.write_sd_config()) {
        set_status_message(F(" CONFIG SAVED, settings have been saved to SD! "));
      } else {
        set_status_message(F(" WRITE CONFIG FAILED, no SD-card! "));
      }
      _current_page = e_sd_MENU_MAX;
      _menu_index = e_sd_page_save_config;
      open_menu(true);
      break;
    case e_sd_MENU_MAX:
      // External entry — open the submenu
      _menu_index = 0;
      open_menu(true);
      break;
    default:
      // Page swap to wipe — render() will show that page
      break;
  }
  force_next_render();
}

void SdSettingsScreen::leave_screen(Controller& controller) {
  _current_page = e_sd_MENU_MAX;
}

BaseScreen* SdSettingsScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  if (menu_open()) {
    return handle_menu_input(controller, workers, SD_MENU, e_sd_MENU_MAX);
  }

  auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);

  if (_current_page == e_sd_page_wipe) {
    if (button2->is_fresh() && button2->get_data().shortPress) {
      M5.Lcd.clear(LCD_COLOR_BACKGROUND);
      M5.Lcd.setRotation(3);
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.setCursor(46, 78, &fonts::Font4);
      M5.Lcd.printf("WIPING SD CARD\n");
      M5.Lcd.setCursor(40, 120, &fonts::Font2);
      M5.Lcd.printf("Removing all log files...\n");
      bool success = SDInterface::i().clear_all_logs();
      M5.Lcd.clear(LCD_COLOR_BACKGROUND);
      M5.Lcd.setRotation(3);
      M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
      M5.Lcd.setCursor(46, 78, &fonts::Font4);
      M5.Lcd.printf(success ? "SD CARD WIPED\n" : "WIPE FAILED\n");
      M5.Lcd.setCursor(70, 130, &fonts::Font2);
      M5.Lcd.printf("Returning to menu...\n");
      delay(1500);
      M5.Lcd.clear(LCD_COLOR_BACKGROUND);
      open_menu(true);
      force_next_render();
      return nullptr;
    }
  }

  if (button3->is_fresh() && button3->get_data().shortPress) {
    open_menu(true);
    M5.Lcd.clear(LCD_COLOR_BACKGROUND);
    force_next_render();
  }
  return nullptr;
}

void SdSettingsScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!force) {
    return;
  }
  if (menu_open()) {
    render_menu(SD_MENU, e_sd_MENU_MAX, true, 1);
    return;
  }
  clear_screen_content();
  if (_current_page == e_sd_page_wipe) {
    render_sd_wipe(workers, handlers);
  }
}

void SdSettingsScreen::render_sd_wipe(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("");
  drawButton2("WIPE");
  drawButton3("Back");

  playErrorBeepsIfAvailable(workers);
  M5.Lcd.fillRect(0, 70, 320, 60, LCD_COLOR_ERROR);
  M5.Lcd.setCursor(10, 85, &fonts::Font2);
  M5.Lcd.setTextColor(TFT_WHITE, LCD_COLOR_ERROR);
  M5.Lcd.printf("WARNING: This will delete ALL log files");
  M5.Lcd.setCursor(10, 105, &fonts::Font2);
  M5.Lcd.printf("from the SD card!");

  M5.Lcd.setCursor(0, 150, &fonts::Font2);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Press WIPE to confirm deleting all log files.\n");
  M5.Lcd.printf("Your device settings will be preserved.\n");
}
