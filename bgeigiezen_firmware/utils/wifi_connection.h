#ifndef BGEIGIEZEN_WIFI_WRAPPER_H_
#define BGEIGIEZEN_WIFI_WRAPPER_H_

class WiFiWrapper {
 public:
  /**
   * Connect to wifi endpoint
   * @param ssid
   * @param password
   * @return true if connected
   */
  bool connect_wifi(const char* ssid, const char* password = nullptr, bool first_time = false);

  /**
   * disconnect from wifi endpoint
   */
  void disconnect_wifi();

  /**
   * check if wifi is connected
   * @return
   */
  bool wifi_connected();

  /**
   * check if wifi is connected
   * @return
   */
  uint8_t status();

  /**
   * Start access point server
   * @return true if up
   */
  bool start_ap_server(const char* host_ssid, const char* password);

  /**
   * Stop access point server
   */
  void stop_ap_server();

  /**
   * check if wifi is connected
   * @return
   */
  bool ap_server_up();

  /**
   * Set wifi hostname
   */
  void set_hostname(const char* hostname, bool ap_mode);

  void update_active();
  bool was_active();

 private:
  uint32_t _last_activity;
};

extern WiFiWrapper WiFiWrapper_i;

#endif //BGEIGIEZEN_WIFI_WRAPPER_H_
