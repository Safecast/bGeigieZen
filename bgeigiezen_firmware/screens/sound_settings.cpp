#include "sound_settings.h"
#include "config_mode.h"
#include "identifiers.h"
#include "user_config.h"
#include "utils/sd_wrapper.h"
#include "workers/local_storage.h"
#include "workers/sound_manager.h"
#include "workers/zen_button.h"

SoundSettingsScreen SoundSettingsScreen_i;

static const SoundSettingsScreen::MenuItem SOUND_MENU[SoundSettingsScreen::e_sound_MENU_MAX] = {
    {.title="Audio settings", .tooltip="Volume, clicks and alarm sound on one page", .enabled=true, .screen=nullptr},
    {.title="CPM alert level", .tooltip="Adjust the CPM alarm threshold level", .enabled=true, .screen=nullptr},
    {.title="Back", .tooltip="Return to Settings", .enabled=true, .screen=&ConfigModeScreen_i},
};

SoundSettingsScreen::SoundSettingsScreen() : BaseScreenWithMenu("Sound", true), _audio_field(0) {
}

void SoundSettingsScreen::enter_screen(Controller& controller) {
  _current_page = e_sound_page_audio;
  _menu_index = 0;
  _audio_field = 0;
  open_menu(true);
  force_next_render();
}

BaseScreen* SoundSettingsScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  if (menu_open()) {
    return handle_menu_input(controller, workers, SOUND_MENU, e_sound_MENU_MAX);
  }

  auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);

  if (_current_page == e_sound_page_audio) {
    if (button1->is_fresh() && button1->get_data().shortPress) {
      _audio_field = (_audio_field + 1) % e_audio_field_MAX;
      force_next_render();
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
      if (_audio_field == e_audio_field_volume) {
        uint8_t current = settings->get_audio_volume();
        uint8_t step = button2->get_data().longPress ? 20 : 5;
        uint16_t next = (uint16_t)current + step;
        uint8_t new_value = (next > 100) ? 0 : (uint8_t)next;
        settings->set_audio_volume(new_value, false);
        if (SDInterface::i().ready()) {
          SDInterface::i().write_safezen_file_from_settings(*settings, false);
        }
        force_next_render();
      } else if (_audio_field == e_audio_field_clicks) {
        auto sound_manager = workers.worker<SoundManager>(k_worker_sound_manager);
        if (sound_manager) {
          sound_manager->toggleSound();
          if (SDInterface::i().ready()) {
            SDInterface::i().write_safezen_file_from_settings(*settings, false);
          }
          force_next_render();
        }
      } else if (_audio_field == e_audio_field_alarm) {
        bool new_setting = !settings->get_error_alert_sound();
        settings->set_error_alert_sound(new_setting, false);
        if (SDInterface::i().ready()) {
          SDInterface::i().write_safezen_file_from_settings(*settings, false);
        }
        force_next_render();
      }
    }
  } else if (_current_page == e_sound_page_cpm_threshold) {
    if (button1->is_fresh() && button1->get_data().shortPress) {
      auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
      uint16_t current_threshold = settings->get_alert_threshold();
      uint16_t decrement = button1->get_data().longPress ? 100 : 10;
      uint16_t new_threshold = (current_threshold <= decrement || current_threshold - decrement < 10)
                                   ? 10
                                   : current_threshold - decrement;
      if (new_threshold != current_threshold) {
        settings->set_alert_threshold(new_threshold, false);
        if (SDInterface::i().ready()) {
          SDInterface::i().write_safezen_file_from_settings(*settings, false);
        }
        force_next_render();
      }
    }
    if (button2->is_fresh() && button2->get_data().shortPress) {
      auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
      uint16_t current_threshold = settings->get_alert_threshold();
      uint16_t increment = button2->get_data().longPress ? 100 : 10;
      uint16_t new_threshold = current_threshold + increment;
      if (new_threshold > 9999) new_threshold = 9999;
      if (new_threshold < 10) new_threshold = 10;
      if (new_threshold != current_threshold) {
        settings->set_alert_threshold(new_threshold, false);
        if (SDInterface::i().ready()) {
          SDInterface::i().write_safezen_file_from_settings(*settings, false);
        }
        force_next_render();
      }
    }
  }

  if (button3->is_fresh() && button3->get_data().shortPress) {
    open_menu(true);
    M5.Lcd.clear(LCD_COLOR_BACKGROUND);
    force_next_render();
  }
  return nullptr;
}

void SoundSettingsScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!force) {
    return;
  }
  if (menu_open()) {
    render_menu(SOUND_MENU, e_sound_MENU_MAX, true, 1);
    return;
  }
  clear_screen_content();
  if (_current_page == e_sound_page_audio) {
    render_audio_page(workers, handlers);
  } else if (_current_page == e_sound_page_cpm_threshold) {
    render_cpm_threshold_page(workers, handlers);
  }
}

void SoundSettingsScreen::render_audio_page(const worker_map_t& workers, const handler_map_t& handlers) {
  auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
  auto sound_manager = workers.worker<SoundManager>(k_worker_sound_manager);

  uint8_t volume = settings->get_audio_volume();
  bool clicks_on = sound_manager && sound_manager->isSoundEnabled();
  bool alarm_on = settings->get_error_alert_sound();

  drawButton1("Next");
  drawButton2(_audio_field == e_audio_field_volume ? "+5%\nHold:+20%" : "Toggle");
  drawButton3("Back");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 45, &fonts::Font2);
  M5.Lcd.printf("Audio Settings\n\n");

  const uint16_t hl_fg = LCD_COLOR_BACKGROUND;
  const uint16_t hl_bg = LCD_COLOR_DEFAULT;
  const uint16_t fg = LCD_COLOR_DEFAULT;
  const uint16_t bg = LCD_COLOR_BACKGROUND;

  M5.Lcd.setTextColor(_audio_field == e_audio_field_volume ? hl_fg : fg,
                      _audio_field == e_audio_field_volume ? hl_bg : bg);
  M5.Lcd.printf(" Volume:  %3u%%       \n", volume);
  M5.Lcd.setTextColor(fg, bg);
  M5.Lcd.printf("\n");

  M5.Lcd.setTextColor(_audio_field == e_audio_field_clicks ? hl_fg : fg,
                      _audio_field == e_audio_field_clicks ? hl_bg : bg);
  M5.Lcd.printf(" Clicks:  %s      \n", clicks_on ? "ON " : "OFF");
  M5.Lcd.setTextColor(fg, bg);
  M5.Lcd.printf("\n");

  M5.Lcd.setTextColor(_audio_field == e_audio_field_alarm ? hl_fg : fg,
                      _audio_field == e_audio_field_alarm ? hl_bg : bg);
  M5.Lcd.printf(" Alarm:   %s      \n", alarm_on ? "ON " : "OFF");
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, bg);
  M5.Lcd.printf("\nA: select  B: change\n");
  M5.Lcd.printf("Volume wraps 100->0\n");
}

void SoundSettingsScreen::render_cpm_threshold_page(const worker_map_t& workers, const handler_map_t& handlers) {
  auto* settings = workers.worker<LocalStorage>(k_worker_local_storage);
  uint16_t current_threshold = settings->get_alert_threshold();

  drawButton1("-10 CPM");
  drawButton2("+10 CPM");
  drawButton3("Back");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(0, 50, &fonts::Font2);
  M5.Lcd.printf("CPM Alert Threshold\n");
  M5.Lcd.printf("\n");
  M5.Lcd.printf("Current: %u CPM\n", current_threshold);
  M5.Lcd.printf("\n");
  M5.Lcd.printf("Press A to decrease by 10 CPM\n");
  M5.Lcd.printf("Press B to increase by 10 CPM\n");
  M5.Lcd.printf("Press C to go back\n");
  M5.Lcd.printf("\n");
  M5.Lcd.setTextColor(LCD_COLOR_STALE_INCOMPLETE, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Hold A: -100 CPM\n");
  M5.Lcd.printf("Hold B: +100 CPM\n");
  M5.Lcd.printf("Range: 10-9999 CPM\n");
}
