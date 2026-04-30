#include "wifi_settings.h"
#include "config_mode.h"
#include "controller.h"
#include "identifiers.h"
#include "user_config.h"
#include "utils/power_manager.h"
#include "utils/wifi_connection.h"
#include "workers/local_storage.h"
#include "workers/zen_button.h"
#include <WiFi.h>

WifiSettingsScreen WifiSettingsScreen_i;

static const WifiSettingsScreen::MenuItem WIFI_MENU[WifiSettingsScreen::e_wifi_MENU_MAX] = {
    {.title="Start Access Point", .tooltip="Start Wi-Fi access point, connect with pc or phone to configure device", .enabled=true, .screen=nullptr},
    {.title="Start on local", .tooltip="Connect to local Wi-Fi, use pc or phone on local network to configure", .enabled=true, .screen=nullptr},
    {.title="Back", .tooltip="Return to Settings", .enabled=true, .screen=&ConfigModeScreen_i},
};

WifiSettingsScreen::WifiSettingsScreen() : BaseScreenWithMenu("WiFi", true) {
  _current_page = e_wifi_MENU_MAX;  // sentinel: "show menu, no action page"
}

void WifiSettingsScreen::enter_screen(Controller& controller) {
  PowerManager::exitLowPowerMode();

  switch (_current_page) {
    case e_wifi_page_ap:
      WiFiWrapper_i.start_ap_server(controller.get_settings().get_device_id(), controller.get_settings().get_ap_password());
      controller.set_worker_active(k_worker_config_server, true);
      break;
    case e_wifi_page_local:
      WiFiWrapper_i.connect_wifi(controller.get_settings().get_wifi_ssid(), controller.get_settings().get_wifi_password());
      controller.set_worker_active(k_worker_config_server, true);
      break;
    default:
      // Sentinel or other — open the submenu
      _menu_index = 0;
      open_menu(true);
      break;
  }
  force_next_render();
}

void WifiSettingsScreen::leave_screen(Controller& controller) {
  switch (_current_page) {
    case e_wifi_page_ap:
      WiFiWrapper_i.stop_ap_server();
      controller.set_worker_active(k_worker_config_server, false);
      break;
    case e_wifi_page_local:
      WiFiWrapper_i.disconnect_wifi();
      controller.set_worker_active(k_worker_config_server, false);
      break;
    default:
      break;
  }
  _current_page = e_wifi_MENU_MAX;
}

BaseScreen* WifiSettingsScreen::handle_input(Controller& controller, const worker_map_t& workers) {
  if (menu_open()) {
    return handle_menu_input(controller, workers, WIFI_MENU, e_wifi_MENU_MAX);
  }
  auto button3 = workers.worker<ZenButton>(k_worker_button_3);
  if (button3->is_fresh() && button3->get_data().shortPress) {
    // Stop the active service before returning to the menu
    if (_current_page == e_wifi_page_ap) {
      WiFiWrapper_i.stop_ap_server();
      controller.set_worker_active(k_worker_config_server, false);
    } else if (_current_page == e_wifi_page_local) {
      WiFiWrapper_i.disconnect_wifi();
      controller.set_worker_active(k_worker_config_server, false);
    }
    _current_page = e_wifi_page_ap;
    open_menu(true);
    M5.Lcd.clear(LCD_COLOR_BACKGROUND);
    force_next_render();
  }
  return nullptr;
}

void WifiSettingsScreen::render(const worker_map_t& workers, const handler_map_t& handlers, bool force) {
  if (!force) {
    return;
  }
  if (menu_open()) {
    render_menu(WIFI_MENU, e_wifi_MENU_MAX, true, 1);
    return;
  }
  clear_screen_content();
  if (_current_page == e_wifi_page_ap) {
    render_page_ap(workers, handlers);
  } else if (_current_page == e_wifi_page_local) {
    render_page_local(workers, handlers);
  }
}

void WifiSettingsScreen::render_page_ap(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("");
  drawButton3("Back");

  const auto settings = workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setCursor(0, 70, &fonts::Font2);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Config through Access Point, connect to\n");
  M5.Lcd.printf("SSID:  %s\n", WiFiWrapper_i.get_hostname());
  M5.Lcd.printf("Password:  %s\n", settings->get_ap_password());

  M5.Lcd.printf("Connect to config page at url :  %s     \n", WiFi.softAPIP().toString().c_str());
}

void WifiSettingsScreen::render_page_local(const worker_map_t& workers, const handler_map_t& handlers) {
  drawButton1("");
  drawButton3("Back");

  const auto settings = workers.worker<LocalStorage>(k_worker_local_storage);

  M5.Lcd.setCursor(0, 70, &fonts::Font2);
  M5.Lcd.setTextColor(LCD_COLOR_DEFAULT, LCD_COLOR_BACKGROUND);
  M5.Lcd.printf("Config through local network, connect to\n");
  M5.Lcd.printf("Network:  %s\n", settings->get_wifi_ssid());
  M5.Lcd.printf("Connected:  %s\n", WiFiWrapper_i.wifi_connected() ? "Yes          " : "Not yet...");

  M5.Lcd.printf("IP:  %s               \n", WiFiWrapper_i.wifi_connected() ? WiFi.localIP().toString().c_str() : "Waiting for connection... ");
}
