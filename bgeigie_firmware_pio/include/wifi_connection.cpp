#include <WiFi.h>
#include <Arduino.h>
#include <ESPmDNS.h>

#include "wifi_connection.h"
#include "debugger.h"


bool WiFiConnection::connect_wifi(const char* ssid, const char* password) {
  if(!ssid) {
    DEBUG_PRINTLN("WiFi connector: No SSID to connect to!");
    return false;
  }
  DEBUG_PRINTLN("WiFi connector: Trying to connect to wifi...");
  switch(WiFi.status()) {
    case WL_CONNECTED:
      return true;
    case WL_DISCONNECTED:
      WiFi.reconnect();
      delay(100);
      return WiFi.isConnected();
    default:
      password ? WiFi.begin(ssid, password) : WiFi.begin(ssid);
      return WiFi.isConnected();
  }
}

void WiFiConnection::disconnect_wifi() {
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_MODE_NULL);
}

bool WiFiConnection::wifi_connected() {
  return WiFi.isConnected();
}

bool WiFiConnection::start_ap_server(const char* host_ssid, const char* password) {
  set_hostname(host_ssid);
  WiFi.softAP(host_ssid, password);
  WiFi.softAPsetHostname(host_ssid);
  delay(100);

  IPAddress ip(ACCESS_POINT_IP);
  IPAddress n_mask(ACCESS_POINT_NMASK);
  WiFi.softAPConfig(ip, ip, n_mask);

  delay(100);

  DEBUG_PRINTF("Access point is up at: %s -> %s\n", host_ssid, WiFi.softAPIP().toString().c_str());
  return true;
}

void WiFiConnection::stop_ap_server() {
  WiFi.softAPdisconnect(true);
  delay(20);
}

bool WiFiConnection::ap_server_up() {
  return (WiFi.getMode() & WIFI_MODE_AP) != 0;
}

void WiFiConnection::set_hostname(const char* hostname) {
  WiFi.setHostname(hostname);
}
