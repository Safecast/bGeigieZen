#include "config_mode.h"
#include "debugger.h"
#include "identifiers.h"
#include "menu_window.h"
#include "user_config.h"
#include "utils/wifi_connection.h"
#include "workers/local_storage.h"
#include "workers/zen_button.h"
#include <WiFi.h>

ConfigModeScreen ConfigModeScreen_i;

ConfigModeScreen::ConfigModeScreen() : BaseScreen("Settings", true) {
}

BaseScreen* ConfigModeScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  auto button1 = workers.worker<ZenButton>(k_worker_button_1);
  auto button2 = workers.worker<ZenButton>(k_worker_button_2);
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);

  if (_options_menu) {
    // Button 1 is move index down
    if (button1->is_fresh() && button1->get_data().shortPress) {
      ++_menu_index;
      _menu_index %= e_config_page_MAX;
      force_next_render();
    }

    // Button 2 move index up
    if (button2->is_fresh() && button2->get_data().shortPress) {
      _menu_index = _menu_index + e_config_page_MAX - 1;
      _menu_index %= e_config_page_MAX;
      force_next_render();
    }

    // Button 3 change view
    if (button3->is_fresh() && button3->get_data().shortPress) {
      _options_menu = false;
      M5.Lcd.clear();
      if (_menu_index != _page) {
        leave_screen(controller);
        _page = static_cast<ConfigModePage>(_menu_index);
        enter_screen(controller);
      }
      force_next_render();
    }
  }
  else {
    if (button1->is_fresh() && button1->get_data().shortPress) {
      _options_menu = true;
      M5.Lcd.clear();
      force_next_render();
    }
    if (button3->is_fresh() && button3->get_data().shortPress) {
      return &MenuWindow_i;
    }
  }

  return nullptr;
}

void ConfigModeScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {

  if (_options_menu) {
    if (!force) {
      return;
    }
    return render_options_menu();
  }

  switch (_page) {
    case e_config_page_main:
      if (!force) {
        return;
      }
      return render_page_main(workers, handlers);
    case e_config_page_ap:
      return render_page_ap(workers, handlers);
    case e_config_page_wifi:
      return render_page_wifi(workers, handlers);
    default:
      return;
  }
}

void ConfigModeScreen::render_options_menu() {
  drawButton1("Down");
  drawButton2("Up");
  drawButton3("Enter");

  // Draw tooltip block
  M5.Lcd.fillRoundRect(161, 26, 142, 158, 4, LCD_COLOR_BACKGROUND);
  M5.Lcd.drawLine(160, 33, 160, 177, LCD_COLOR_STALE_INCOMPLETE);

  for (int i = 0; i < e_config_page_MAX; ++i) {
    M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
    M5.Lcd.drawLine(16, 48 + (i * 16), 159, 48 + (i * 16), (i == _menu_index ? LCD_COLOR_STALE_INCOMPLETE : LCD_COLOR_BACKGROUND));
    M5.Lcd.setCursor(16, 40 + (i * 16), 2);
    if (i == _menu_index) {
      M5.Lcd.print("> ");
    }
    else {
      M5.Lcd.print("  ");
    }
    switch (i) {
      case e_config_page_main:
        M5.Lcd.print("Current settings ");
        break;
      case e_config_page_ap:
        M5.Lcd.print("Start Access Point ");
        break;
      case e_config_page_wifi:
        M5.Lcd.print("Start on local ");
        break;
      case e_config_page_load_sd_config:
        M5.Lcd.print("Load config from SD ");
        break;
      case e_config_page_save_config_to_sd:
        M5.Lcd.print("Save config to SD ");
        break;
      case e_config_page_reset:
        M5.Lcd.print("Reset device ");
        break;
      default:
        break;
    }
  }

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.setCursor(170, 30);
  M5.Lcd.print("title option");
  M5.Lcd.println(_menu_index);
  M5.Lcd.setCursor(170, 60);

  M5.Lcd.setCursor(0, 0, 1);
}

void ConfigModeScreen::render_page_main(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("Options");
  drawButton3("Menu");

  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);

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

void ConfigModeScreen::render_page_ap(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("Options");
  drawButton3("Menu");

  const auto settings = workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setCursor(0, 78, 1);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Config through Access Point, connect to\n");
  M5.Lcd.printf("SSID:        %s\n", WiFiWrapper_i.get_hostname());
  M5.Lcd.printf("Password:    %s\n", settings->get_ap_password());

  M5.Lcd.printf("IP:          %s     \n", WiFi.softAPIP().toString().c_str());

}

void ConfigModeScreen::render_page_wifi(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("Options");
  drawButton3("Menu");

  const auto settings = workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setCursor(0, 78, 1);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Config through local network, connect to\n");
  M5.Lcd.printf("Network:     %s\n", settings->get_wifi_ssid());
  M5.Lcd.printf("Connected:   %s\n", WiFiWrapper_i.wifi_connected() ? "Yes" : "Not yet...");

  M5.Lcd.printf("IP:          %s       \n", WiFiWrapper_i.wifi_connected() ? WiFi.localIP().toString().c_str() : "Waiting for connection... ");
}

void ConfigModeScreen::enter_screen(Controller& controller) {
  DEBUG_PRINTF("enter_screen config %d\n", _page);
  switch (_page) {
    case e_config_page_ap:
      WiFiWrapper_i.start_ap_server(controller.get_settings().get_device_id(), controller.get_settings().get_ap_password());
      controller.set_worker_active(k_worker_config_server, true);
      break;
    case e_config_page_wifi:
      WiFiWrapper_i.connect_wifi(controller.get_settings().get_wifi_ssid(), controller.get_settings().get_wifi_password());
      controller.set_worker_active(k_worker_config_server, true);
      break;
    default:
      break;
  }
}

void ConfigModeScreen::leave_screen(Controller& controller) {
  DEBUG_PRINTF("leave_screen config %d\n", _page);
  switch (_page) {
    case e_config_page_ap:
      WiFiWrapper_i.stop_ap_server();
      controller.set_worker_active(k_worker_config_server, false);
      break;
    case e_config_page_wifi:
      WiFiWrapper_i.disconnect_wifi();
      controller.set_worker_active(k_worker_config_server, false);
      break;
    default:
      break;
  }

  // Set page to main in case we left through main menu
  _page = e_config_page_main;
}
