#ifndef BGEIGIECAST_WIFI_CONNECTION_H_
#define BGEIGIECAST_WIFI_CONNECTION_H_

class WiFiConnection {
 public:
  /**
   * Connect to wifi endpoint
   * @param ssid
   * @param password
   * @return true if connected
   */
  static bool connect_wifi(const char* ssid, const char* password = nullptr);

  /**
   * disconnect from wifi endpoint
   */
  static void disconnect_wifi();

  /**
   * check if wifi is connected
   * @return
   */
  static bool wifi_connected();

  /**
   * Start access point server
   * @return true if up
   */
  static bool start_ap_server(const char* host_ssid, const char* password);

  /**
   * Stop access point server
   */
  static void stop_ap_server();

  /**
   * check if wifi is connected
   * @return
   */
  static bool ap_server_up();

  /**
   * Set wifi hostname
   */
  static void set_hostname(const char* hostname);
};

#endif //BGEIGIECAST_WIFI_CONNECTION_H_
