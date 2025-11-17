#include <WiFi.h>
#include <Arduino.h>
#include <ESPmDNS.h>
#include <M5Unified.h>
#include <esp_wifi.h>

#include "user_config.h"
#include "wifi_connection.h"

WiFiWrapper WiFiWrapper_i;


WiFiWrapper::WiFiWrapper(): _last_activity(0), _hostname("") {
}


bool WiFiWrapper::connect_wifi(const char* ssid, const char* password, bool first_time) {
  switch(WiFi.status()) {
    case WL_CONNECTED:
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
        delay(100);
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
      delay(100);
      update_active();
      return wifi_connected();
    default:
      M5_LOGD("WiFi connector: Trying to connect to wifi (%s:%s)...", ssid, password);
      #ifndef CONFIG_IDF_TARGET_ESP32S3
      // On Core2, ensure clean WiFi state before connecting to new network
      if (WiFi.getMode() != WIFI_MODE_NULL) {
        WiFi.disconnect(true);
        // Core2: Don't set mode - just ensure disconnected state
        delay(100);
      }
      #endif
      password ? WiFi.begin(ssid, password) : WiFi.begin(ssid);
      delay(100);
      update_active();
      return wifi_connected();
  }
}

void WiFiWrapper::disconnect_wifi() {
  #ifdef CONFIG_IDF_TARGET_ESP32S3
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_MODE_NULL);  // CoreS3 can safely deinit WiFi
  #else
    // Core2: NEVER call WiFi.mode() as it triggers deinit
    // Just disconnect - driver stays initialized
    WiFi.disconnect(true);  // Disconnect but don't erase credentials
    esp_wifi_stop();        // Stop WiFi radio
    M5_LOGD("Core2: WiFi stopped but driver kept initialized");
  #endif
}

bool WiFiWrapper::wifi_connected() {
  return WiFi.isConnected();
}

uint8_t WiFiWrapper::status() {
  return WiFi.status();
}

bool WiFiWrapper::start_ap_server(uint16_t device_id, const char* password) {
  #ifndef CONFIG_IDF_TARGET_ESP32S3
  // On Core2, ensure WiFi is in clean state before starting AP
  M5_LOGD("Core2: Ensuring clean WiFi state before AP start");

  // Disconnect if currently connected, but don't force deinit
  if (WiFi.getMode() != WIFI_MODE_NULL) {
    WiFi.disconnect(true);
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    WiFi.mode(WIFI_OFF);  // CoreS3 can safely deinit WiFi
    #else
    // Core2: NEVER call WiFi.mode() as it triggers deinit
    esp_wifi_stop();        // Just stop the radio
    #endif
    delay(100);
  }
  #endif

  char host_ssid[20];
  sprintf(host_ssid, ACCESS_POINT_SSID, device_id);
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
