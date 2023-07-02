#include <Update.h>
#include <ESPmDNS.h>
#include <configuration_server.hpp>
#include "user_config.h"
#include "local_storage.h"
#include "debugger.h"
#include "http_pages.h"
#include "identifiers.h"

#define RETRY_TIMEOUT 4000

template<typename T, typename T2>
T clamp(T2 val, T min, T max) {
  T _val = static_cast<T>(val);
  return _val < min ? min : _val > max ? max : _val;
}

ConfigWebServer::ConfigWebServer(Display& display)
      _server(SERVER_WIFI_PORT),
      // _config(config)
      {
  add_urls();
}

bool ConfigWebServer::activate(bool) {
  if(WiFiConnection::wifi_connected() || WiFiConnection::ap_server_up()) {
    // Set DNS hostname for easy access
    char hostname[16];
    sprintf(hostname, ACCESS_POINT_SSID, display.data.device_id);
    if(MDNS.begin(hostname)) {
      WiFiConnection::set_hostname(hostname);
      MDNS.addService("http", "tcp", 80);
    }

    // Start config server
    _server.begin(SERVER_WIFI_PORT);
    HttpPages::internet_access = WiFiConnection::wifi_connected();
    return true;
  }
  return false;
}

void ConfigWebServer::deactivate() {
  _server.close();
  _server.stop();
  MDNS.end();
}

int8_t ConfigWebServer::produce_data() {
  _server.handleClient();
  if(data == k_server_status_offline) {
    data = HttpPages::internet_access ? k_server_status_running_wifi : k_server_status_running_access_point;
    return WorkerStatus::e_worker_data_read;
  }
  return WorkerStatus::e_worker_idle;
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
        _server.hasArg("success")
        // _config.get_device_id(),
        // _config.get_led_color_intensity(),
        // _config.is_led_color_blind()
    ));
  });

  // Configure Connection
  _server.on("/connection", HTTP_GET, [this]() {
    _server.sendHeader("Connection", "close");
    _server.send(200, "text/html", HttpPages::get_config_connection_page(
        _server.hasArg("success"),
        _config.get_device_id(),
        _config.get_ap_password(),
        _config.get_wifi_ssid(),
        _config.get_wifi_password(),
        _config.get_api_key(),
        _config.get_use_dev()
    ));
  });

  // Configure Location
  _server.on("/location", HTTP_GET, [this]() {

    _server.sendHeader("Connection", "close");
    _server.send(200, "text/html", HttpPages::get_config_location_page(
        _server.hasArg("success"),
        _config.get_device_id(),
        _config.get_use_home_location(),
        _config.get_home_latitude(),
        _config.get_home_longitude(),
        _config.get_last_latitude(),
        _config.get_last_longitude()
    ));

    _server.sendHeader("Connection", "close");
  });

  // Save config
  _server.on("/save", HTTP_POST, [this]() {
    handle_save();
  });

  // Upload get
  _server.on("/update", HTTP_GET, [this]() {
    _server.sendHeader("Connection", "close");
    _server.send(200, "text/html", HttpPages::get_update_page(_config.get_device_id()));
  });

  // Status get
  _server.on("/status", HTTP_GET, [this]() {
    _server.sendHeader("Connection", "close");
    _server.send(200, "text/html", HttpPages::get_status_page(_config.get_device_id()));
  });

  // Upload post
  _server.on("/update", HTTP_POST, [this]() {
    // Complete
    _server.sendHeader("Connection", "close");
    if(_server.upload().totalSize == 0 || Update.hasError()) {
      _server.send(500, "text/plain", "FAIL");
    } else {
      _server.send(200, "text/plain", "OK");
      _server.client().flush();
    }
  }, [this]() {
    // Progress
    handle_update_uploading();
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
  if(_server.hasArg(FORM_NAME_USE_DEV)) {
    _config.set_use_dev(_server.arg(FORM_NAME_USE_DEV) == "1", false);
  }
  if(_server.hasArg(FORM_NAME_LED_INTENSITY)) {
    _config.set_led_color_intensity(clamp<uint8_t>(_server.arg(FORM_NAME_LED_INTENSITY).toInt(), 5, 100), false);
  }
  if(_server.hasArg(FORM_NAME_LED_COLOR)) {
    _config.set_led_color_blind(strcmp(_server.arg(FORM_NAME_LED_COLOR).c_str(), "1") == 0, false);
  }
  if(_server.hasArg(FORM_NAME_LOC_HOME)) {
    _config.set_use_home_location(strcmp(_server.arg(FORM_NAME_LOC_HOME).c_str(), "1") == 0, false);
  }
  if(_server.hasArg(FORM_NAME_LOC_HOME_LAT)) {
    _config.set_home_latitude(clamp<double>(_server.arg(FORM_NAME_LOC_HOME_LAT).toDouble(), -90.0, 90.0), false);
  }
  if(_server.hasArg(FORM_NAME_LOC_HOME_LON)) {
    _config.set_home_longitude(clamp<double>(_server.arg(FORM_NAME_LOC_HOME_LON).toDouble(), -180.0, 180.0), false);
  }

  _server.sendHeader("Location", _server.arg("next") + "?success=true");
  _server.send(302, "text/html");
  _server.client().flush();
}

void ConfigWebServer::handle_update_uploading() {
  HTTPUpload& upload = _server.upload();
  switch(upload.status) {
    case UPLOAD_FILE_START: {
      DEBUG_PRINTF("Update: %s\n", upload.filename.c_str());
      if(Update.begin(UPDATE_SIZE_UNKNOWN)) {
        DEBUG_PRINTLN("Starting update");
      } else {
        DEBUG_PRINTLN("Unable to start update");
      }
      break;
    }
    case UPLOAD_FILE_WRITE: {
      DEBUG_PRINTF(".");
      auto write_size = Update.write(upload.buf, upload.currentSize);
      if(write_size != upload.currentSize) {
        DEBUG_PRINTF("Something failed while uploading (wrote %d out of %d)\n", write_size, upload.currentSize);
        Update.abort();
      }
      break;
    }
    case UPLOAD_FILE_END: {
      if(upload.totalSize > 0 && Update.end(true)) { //true to set the size to the current progress
        DEBUG_PRINTF("\nUpdate Success: %u\n", upload.totalSize);
      } else {
        DEBUG_PRINTF("Update Failed...");
      }
      break;
    }
    case UPLOAD_FILE_ABORTED: {
      Update.abort();
      DEBUG_PRINTF("Update aborted...");
      break;
    }
  }
}

void ConfigWebServer::handle_report(const Report& report) {
}
