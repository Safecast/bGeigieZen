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
      _config(config),
      _handled_client(false) {
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
    data = HttpPages::internet_access ? k_server_status_running_wifi : k_server_status_running_access_point;
    return true;
  }
  return false;
}

void ConfigWebServer::deactivate() {
  data = k_server_status_offline;
  _server.close();
}

int8_t ConfigWebServer::produce_data() {
  _server.handleClient();
  if (_handled_client) {
    _handled_client = false;
    return Worker::e_worker_data_read;
  }
  return Worker::e_worker_idle;
}

void ConfigWebServer::add_urls() {
  // Home
  _server.on("/", HTTP_GET, [this]() {
    M5_LOGD("Handle get home");
    _server.sendHeader("Connection", "close");
    _server.send_P(200, "text/html", HttpPages::get_home_page(_config));
    _handled_client = true;
  });

  // Configure Device
  _server.on("/device", HTTP_GET, [this]() {
    M5_LOGD("Handle get device settings");
    _server.sendHeader("Connection", "close");
    _server.send_P(200, "text/html", HttpPages::get_config_device_page(
        _server.hasArg("success"),
        _config
    ));
    _handled_client = true;
  });

  // Configure Connection
  _server.on("/connection", HTTP_GET, [this]() {
    M5_LOGD("Handle get connection settings");
    _server.sendHeader("Connection", "close");
    _server.send_P(200, "text/html", HttpPages::get_config_connection_page(
        _server.hasArg("success"),
        _config
    ));
    _handled_client = true;
  });

  // Configure Location
  _server.on("/location", HTTP_GET, [this]() {
    M5_LOGD("Handle get location settings");
    _server.sendHeader("Connection", "close");
    _server.send_P(200, "text/html", HttpPages::get_config_location_page(
        _server.hasArg("success"),
        _config
    ));
    _handled_client = true;
  });

  // Save config
  _server.on("/save", HTTP_POST, [this]() {
    handle_save();
    _handled_client = true;
  });

  _server.on("/reboot", [this]() { // Reboot
    M5_LOGD("Handle reboot request");
    _server.send_P(200, "text/plain", "OK");
    _server.client().flush();
    ESP.restart();
    _handled_client = true;
  });

  // css get
  _server.on("/pure.css", HTTP_GET, [this]() {
    _server.sendHeader("Content-Encoding", "gzip");
    _server.send_P(200, "text/css", reinterpret_cast<const char*>(HttpPages::pure_css), PURE_CSS_SIZE);
    _handled_client = true;
  });

  // css get
  _server.on("/favicon.ico", HTTP_GET, [this]() {
    _server.send_P(200, "image/x-icon", reinterpret_cast<const char*>(HttpPages::favicon), FAVICON_SIZE);
    _handled_client = true;
  });
}

void ConfigWebServer::handle_save() {
//  if(_server.hasArg(...)) {
//    _config.set...(_server.arg(...).c_str(), ...);
//  }

  M5_LOGD("Handle post request");

  if(_server.hasArg(FORM_NAME_ALERT_THRESHOLD)) {
    _config.set_alert_threshold(clamp<uint16_t>(_server.arg(FORM_NAME_ALERT_THRESHOLD).toInt(), 0, 34464), false);
  }
  if(_server.hasArg(FORM_NAME_SCREEN_DIM_TIMEOUT)) {
    _config.set_screen_dim_timeout(clamp<uint16_t>(_server.arg(FORM_NAME_SCREEN_DIM_TIMEOUT).toInt(), 0, 34464), false);
  }
  if(_server.hasArg(FORM_NAME_SCREEN_OFF_TIMEOUT)) {
    _config.set_screen_off_timeout(clamp<uint16_t>(_server.arg(FORM_NAME_SCREEN_OFF_TIMEOUT).toInt(), 0, 34464), false);
  }
  if(_server.hasArg(FORM_NAME_ANIMATED_SCREENSAVER)) {
    _config.set_animated_screensaver(_server.arg(FORM_NAME_ANIMATED_SCREENSAVER).toInt(), false);
  }
  if(_server.hasArg(FORM_NAME_CPM_USVH)) {
    _config.set_cpm_usvh(_server.arg(FORM_NAME_CPM_USVH).toInt(), false);
  }
  if(_server.hasArg(FORM_NAME_MANUAL_LOGGING)) {
    _config.set_manual_logging(_server.arg(FORM_NAME_MANUAL_LOGGING).toInt(), false);
  }
  if(_server.hasArg(FORM_NAME_ENABLE_JOURNAL)) {
    _config.set_enable_journal(_server.arg(FORM_NAME_ENABLE_JOURNAL).toInt(), false);
  }
  if(_server.hasArg(FORM_NAME_LOG_VOID)) {
    _config.set_log_void(_server.arg(FORM_NAME_LOG_VOID).toInt(), false);
  }
  if(_server.hasArg(FORM_NAME_AP_LOGIN)) {
    _config.set_ap_password(_server.arg(FORM_NAME_AP_LOGIN).c_str(), false);
  }
  if(_server.hasArg(FORM_NAME_WIFI_SSID)) {
    _config.set_wifi_ssid(_server.arg(FORM_NAME_WIFI_SSID).c_str(), false);
  }
  if(_server.hasArg(FORM_NAME_WIFI_PASS)) {
    _config.set_wifi_password(_server.arg(FORM_NAME_WIFI_PASS).c_str(), false);
  }
  if(_server.hasArg(FORM_NAME_API_KEY)) {
    _config.set_api_key(_server.arg(FORM_NAME_API_KEY).c_str(), false);
  }
  if(_server.hasArg(FORM_NAME_LOC_FIXED_LAT)) {
    _config.set_fixed_latitude(clamp<double>(_server.arg(FORM_NAME_LOC_FIXED_LAT).toDouble(), -90.0, 90.0), false);
  }
  if(_server.hasArg(FORM_NAME_LOC_FIXED_LON)) {
    _config.set_fixed_longitude(clamp<double>(_server.arg(FORM_NAME_LOC_FIXED_LON).toDouble(), -180.0, 180.0), false);
  }
  if(_server.hasArg(FORM_NAME_FIXED_RANGE)) {
    _config.set_fixed_range(clamp<float>(_server.arg(FORM_NAME_FIXED_RANGE).toFloat(), 0, 5.0), false);
  }
  if(_server.hasArg(FORM_NAME_DOP_MAX)) {
    _config.set_dop_max(clamp<uint16_t>(_server.arg(FORM_NAME_DOP_MAX).toInt(), 0, 5000), false);
  }

  _server.sendHeader("Location", _server.arg("next") + "?success=true");
  _server.send(302, "text/html");
  _server.client().flush();
}
