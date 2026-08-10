#include <WiFi.h>
#include <Arduino.h>
#include <ESPmDNS.h>
#include <M5Unified.h>
#include <esp_wifi.h>

#include "user_config.h"
#include "wifi_connection.h"

WiFiWrapper WiFiWrapper_i;

#define WIFI_CONNECT_TIMEOUT_MS 8000

static void on_wifi_event(WiFiEvent_t event) {
  if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    M5_LOGD("WiFi connector: STA disconnected event received");
    WiFiWrapper_i.flag_disconnect_event();
  }
}

WiFiWrapper::WiFiWrapper(): _last_activity(0), _hostname(""), _disconnect_event(false),
    _connect_attempt_active(false), _connect_timer(WIFI_CONNECT_TIMEOUT_MS) {
}

void WiFiWrapper::register_events() {
  WiFi.onEvent(on_wifi_event);
}

void WiFiWrapper::flag_disconnect_event() {
  _disconnect_event = true;
}

bool WiFiWrapper::consume_disconnect_event() {
  bool fired = _disconnect_event;
  _disconnect_event = false;
  return fired;
}


bool WiFiWrapper::connect_wifi(const char* ssid, const char* password, bool first_time) {
  switch(WiFi.status()) {
    case WL_CONNECTED:
      _connect_attempt_active = false;
      return true;
    case WL_CONNECT_FAILED:
      if (first_time) {
        M5_LOGD("WiFi connector: Trying to reconnect to wifi...");
        #ifndef CONFIG_IDF_TARGET_ESP32S3
        // On Core2, disconnect and clear state before reconnecting
        WiFi.disconnect(true);
        delay(50);
        #endif
        WiFi.reconnect();
        _connect_attempt_active = true;
        _connect_timer.restart();
        update_active();
        return wifi_connected();
      }
      return false;
    case WL_DISCONNECTED:
      M5_LOGD("WiFi connector: Trying to reconnect to wifi...");
      #ifndef CONFIG_IDF_TARGET_ESP32S3
      // On Core2, disconnect and clear state before reconnecting
      WiFi.disconnect(true);
      delay(50);
      #endif
      WiFi.reconnect();
      _connect_attempt_active = true;
      _connect_timer.restart();
      update_active();
      return wifi_connected();
    default:
      M5_LOGD("WiFi connector: Trying to connect to wifi (%s:%s)...", ssid, password);
      #ifndef CONFIG_IDF_TARGET_ESP32S3
      // On Core2, ensure WiFi is properly started before connecting
      uint8_t current_status = WiFi.status();
      M5_LOGD("Core2: Current WiFi status before connect: %d", current_status);

      if (current_status == 255) {
        // WiFi is completely uninitialized - initialize and start it
        M5_LOGD("Core2: WiFi uninitialized (status 255), initializing and starting WiFi");
        WiFi.mode(WIFI_STA);
        delay(100);
        esp_wifi_start();  // Explicitly start the WiFi radio
        delay(200);
        current_status = WiFi.status();
        M5_LOGD("Core2: WiFi started, status now: %d", current_status);
      } else if (current_status == WL_DISCONNECTED || current_status == WL_IDLE_STATUS) {
        // WiFi is initialized but disconnected - safe to restart
        M5_LOGD("Core2: Restarting WiFi radio from status %d", current_status);
        esp_wifi_stop();  // Ensure it's fully stopped
        delay(100);
        esp_wifi_start(); // Restart it
        delay(200);
        current_status = WiFi.status();
        M5_LOGD("Core2: WiFi status after restart: %d", current_status);
      } else if (current_status == WL_CONNECTED) {
        // Only disconnect if currently connected to a different network
        String current_ssid = WiFi.SSID();
        if (current_ssid != ssid) {
          M5_LOGD("Core2: Disconnecting from different network: %s", current_ssid.c_str());
          WiFi.disconnect();
          delay(200);
        }
      }
      #endif
      password ? WiFi.begin(ssid, password) : WiFi.begin(ssid);
      _connect_attempt_active = true;
      _connect_timer.restart();
      update_active();
      return wifi_connected();
  }
}

bool WiFiWrapper::connecting() {
  if (wifi_connected()) {
    _connect_attempt_active = false;
    return false;
  }
  return _connect_attempt_active && !_connect_timer.isExpired();
}

bool WiFiWrapper::connect_timed_out() {
  if (wifi_connected()) {
    return false;
  }
  return _connect_attempt_active && _connect_timer.isExpired();
}

void WiFiWrapper::disconnect_wifi() {
  #ifdef CONFIG_IDF_TARGET_ESP32S3
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_MODE_NULL);  // CoreS3 can safely deinit WiFi
  #else
  // Core2: Use soft disconnect to avoid WiFi reinitialization issues
  // disconnect(wifioff=false, eraseap=false) - disconnects but keeps WiFi running
  WiFi.disconnect(false, false);
  M5_LOGD("Core2: WiFi disconnected (soft) - driver remains active");
  #endif
}

bool WiFiWrapper::wifi_connected() {
  return WiFi.isConnected();
}

uint8_t WiFiWrapper::status() {
  return WiFi.status();
}

bool WiFiWrapper::start_ap_server(uint16_t device_id, const char* password) {
  M5_LOGD("Starting WiFi AP server...");

  wifi_mode_t current_mode = WiFi.getMode();
  M5_LOGD("Current WiFi mode: %d", current_mode);

  #ifndef CONFIG_IDF_TARGET_ESP32S3
  // On Core2, we need to ensure WiFi is properly initialized and reset before AP
  // to avoid buffer allocation errors during mode operations
  if (current_mode == WIFI_MODE_NULL) {
    // WiFi is completely uninitialized - initialize it first
    M5_LOGD("Core2: WiFi uninitialized, initializing to STA mode first");
    WiFi.mode(WIFI_STA);
    delay(200);
    current_mode = WiFi.getMode();
    M5_LOGD("Core2: WiFi initialized, mode now: %d", current_mode);
  }

  // Now do a full reset regardless of previous state to ensure clean AP initialization
  if (current_mode == WIFI_STA || current_mode == WIFI_AP_STA) {
    M5_LOGD("Core2: Fully resetting WiFi from mode %d to AP mode", current_mode);
    WiFi.disconnect(false);
    delay(200);

    // Fully stop WiFi to reset buffers
    esp_wifi_stop();
    delay(300);

    // Restart WiFi
    esp_wifi_start();
    delay(300);

    M5_LOGD("Core2: WiFi reset complete, mode now: %d", WiFi.getMode());
  }
  #else
  // CoreS3: Can use standard disconnect
  if (current_mode != WIFI_MODE_NULL && current_mode != WIFI_AP) {
    WiFi.disconnect(true);
    delay(200);
  }
  #endif

  // softAP() will now initialize in AP mode with fresh buffers
  char host_ssid[20];
  sprintf(host_ssid, ACCESS_POINT_SSID, device_id);
  M5_LOGD("Starting AP with SSID: %s", host_ssid);
  WiFi.softAP(host_ssid, password);
  WiFi.softAPsetHostname(host_ssid);
  delay(100);

  IPAddress ip(ACCESS_POINT_IP);
  IPAddress n_mask(ACCESS_POINT_NMASK);
  WiFi.softAPConfig(ip, ip, n_mask);

  delay(100);

  M5_LOGD("Access point is up at: %s -> %s", host_ssid, WiFi.softAPIP().toString().c_str());
  return true;
}

void WiFiWrapper::stop_ap_server() {
  if (ap_server_up()) {
    WiFi.softAPdisconnect(true);
    delay(20);
  }
}

bool WiFiWrapper::ap_server_up() {
  return (WiFi.getMode() & WIFI_MODE_AP) != 0;
}

void WiFiWrapper::set_hostname(const char* hostname, bool ap_mode) {
  strcpy(_hostname, hostname);
  WiFi.setHostname(hostname);
  if (ap_mode) {
    if(MDNS.begin(hostname)) {
      MDNS.addService("http", "tcp", 80);
    }
  }
}

const char* WiFiWrapper::get_hostname() const {
  return _hostname;
}

void WiFiWrapper::update_active() {
  if (wifi_connected()) {
    WiFiWrapper::_last_activity = millis();
  }
}

bool WiFiWrapper::was_active() {
  return WiFiWrapper::_last_activity && (WiFiWrapper::_last_activity + 500) > millis();
}
