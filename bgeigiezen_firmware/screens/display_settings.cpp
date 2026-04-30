#include "display_settings.h"
#include "config_mode.h"
#include "identifiers.h"
#include "user_config.h"
#include "utils/sd_wrapper.h"
#include "workers/local_storage.h"
#include "workers/zen_button.h"

DisplaySettingsScreen DisplaySettingsScreen_i;

static const DisplaySettingsScreen::MenuItem DISPLAY_MENU[DisplaySettingsScreen::e_display_MENU_MAX] = {
    {.title="Dim brightness", .tooltip="Adjust screen brightness when dimmed/screensaver", .enabled=true, .screen=nullptr},
    {.title="Back", .tooltip="Return to Settings", .enabled=true, .screen=&ConfigModeScreen_i},
};

DisplaySettingsScreen::DisplaySettingsScreen() : BaseScreenWithMenu("Display", true) {
}

void DisplaySettingsScreen::enter_screen(Controller& controller) {
  _current_page = e_display_page_dim_brightness;
  _menu_index = 0;
  open_menu(true);
  force_next_render();
}

BaseScreen* DisplaySettingsScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  if (menu_open()) {
    return handle_menu_input(controller, workers, DISPLAY_MENU, e_display_MENU_MAX);
  }

  auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);

  if (_current_page == e_display_page_dim_brightness) {
    if (button1->is_fresh() && button1->get_data().shortPress) {
      auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
      uint8_t current = settings->get_dim_brightness();
      uint8_t step = button1->get_data().longPress ? 20 : 5;
      uint8_t new_value = (current > step) ? (uint8_t)(current - step) : 0;
      if (new_value != current) {
        settings->set_dim_brightness(new_value, false);
        if (SDInterface::i().ready()) {
          SDInterface::i().write_safezen_file_from_settings(*settings, false);
        }
        force_next_render();
      }
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
      uint8_t current = settings->get_dim_brightness();
      uint8_t step = button2->get_data().longPress ? 20 : 5;
      uint16_t temp = current + step;
      uint8_t new_value = (temp > 100) ? 100 : (uint8_t)temp;
      if (new_value != current) {
        settings->set_dim_brightness(new_value, false);
        if (SDInterface::i().ready()) {
          SDInterface::i().write_safezen_file_from_settings(*settings, false);
        }
        force_next_render();
      }
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      open_menu(true);
      M5.Lcd.clear(LCD_COLOR_BACKGROUND);
      force_next_render();
    }
  }
  return nullptr;
}

void DisplaySettingsScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!force) {
    return;
  }
  if (menu_open()) {
    render_menu(DISPLAY_MENU, e_display_MENU_MAX, true, 1);
    return;
  }
  clear_screen_content();
  if (_current_page == e_display_page_dim_brightness) {
    render_dim_brightness_page(workers, handlers);
  }
}

void DisplaySettingsScreen::render_dim_brightness_page(const worker_map_t& workers, const handler_map_t& handlers) {
  auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
  uint8_t current = settings->get_dim_brightness();

  drawButton1("-5%\nHold:-20%");
  drawButton2("+5%\nHold:+20%");
  drawButton3("Back");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 50, &fonts::Font2);
  M5.Lcd.printf("Dim Brightness\n\n");
  M5.Lcd.printf("Current: %u%%\n\n", current);
  M5.Lcd.printf("Controls the screen brightness used\n");
  M5.Lcd.printf("when the device is DIM or the\n");
  M5.Lcd.printf("animated screensaver is active.\n\n");
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Range: 0-100%%\n");
}
