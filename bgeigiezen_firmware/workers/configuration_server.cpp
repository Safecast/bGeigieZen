#include "configuration_server.h"
#include "user_config.h"
#include "utils/http_pages.h"

#define RETRY_TIMEOUT 4000

template<typename T, typename T2>
T clamp(T2 val, T min, T max) {
  T _val = static_cast<T>(val);
  return _val < min ? min : _val > max ? max : _val;
}

ConfigWebServer::ConfigWebServer(LocalStorage& config)
    : Worker<ServerStatus>(k_server_status_offline, 0),
      _server(SERVER_WIFI_PORT),
      _config(config) {
  add_urls();
}

bool ConfigWebServer::activate(bool) {
  if(WiFiWrapper_i.wifi_connected() || WiFiWrapper_i.ap_server_up()) {
    // Set DNS hostname for easy access
    char hostname[20];
    sprintf(hostname, ACCESS_POINT_SSID, _config.get_device_id());
    WiFiWrapper_i.set_hostname(hostname, true);

    // Start config server
    _server.begin(SERVER_WIFI_PORT);
    HttpPages::internet_access = WiFiWrapper_i.wifi_connected();
    return true;
  }
  return false;
}

void ConfigWebServer::deactivate() {
  _server.close();
  _server.stop();
}

int8_t ConfigWebServer::produce_data() {
  _server.handleClient();
  if(data == k_server_status_offline) {
    data = HttpPages::internet_access ? k_server_status_running_wifi : k_server_status_running_access_point;
    return Worker::e_worker_data_read;
  }
  return Worker::e_worker_idle;
}

void ConfigWebServer::add_urls() {
  // Home
  _server.on("/", HTTP_GET, [this]() {
    _server.sendHeader("Connection", "close");
    _server.send(200, "text/html", HttpPages::get_home_page(_config.get_device_id()));
  });

  // Configure Device
  _server.on("/device", HTTP_GET, [this]() {
    _server.sendHeader("Connection", "close");
    _server.send(200, "text/html", HttpPages::get_config_device_page(
        _server.hasArg("success"),
        _config.get_device_id()
    ));
  });

  // Configure Connection
  _server.on("/connection", HTTP_GET, [this]() {
    _server.sendHeader("Connection", "close");
    _server.send(200, "text/html", HttpPages::get_config_connection_page(
        _server.hasArg("success"),
        _config.get_device_id()
    ));
  });

  // Configure Location
  _server.on("/location", HTTP_GET, [this]() {

    _server.sendHeader("Connection", "close");
    _server.send(200, "text/html", HttpPages::get_config_location_page(
        _server.hasArg("success"),
        _config.get_device_id()
    ));

    _server.sendHeader("Connection", "close");
  });

  // Save config
  _server.on("/save", HTTP_POST, [this]() {
    handle_save();
  });

  _server.on("/reboot", [this]() { // Reboot
    _server.send(200, "text/plain", "OK");
    _server.client().flush();
    ESP.restart();
  });

  // css get
  _server.on("/pure.css", HTTP_GET, [this]() {
    _server.sendHeader("Content-Encoding", "gzip");
    _server.send_P(200, "text/css", reinterpret_cast<const char*>(HttpPages::pure_css), PURE_CSS_SIZE);
  });

  // css get
  _server.on("/favicon.ico", HTTP_GET, [this]() {
    _server.send_P(200, "image/x-icon", reinterpret_cast<const char*>(HttpPages::favicon), FAVICON_SIZE);
  });
}

void ConfigWebServer::handle_save() {
//  if(_server.hasArg(...)) {
//    _config.set...(_server.arg(...).c_str(), ...);
//  }

  _server.sendHeader("Location", _server.arg("next") + "?success=true");
  _server.send(302, "text/html");
  _server.client().flush();
}
