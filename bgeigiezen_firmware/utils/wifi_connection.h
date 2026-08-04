#ifndef BGEIGIEZEN_WIFI_WRAPPER_H_
#define BGEIGIEZEN_WIFI_WRAPPER_H_

class WiFiWrapper {
 public:
  WiFiWrapper();
  /**
   * Connect to wifi endpoint
   * @param ssid
   * @param password
   * @param wait_for_result if true, block until connected or a bounded
   *        timeout elapses (safe for user-initiated UI actions); if false,
   *        kick off the connection attempt and return immediately without
   *        blocking the caller (required for calls made from the main loop,
   *        so touch/button polling isn't starved).
   * @return true if connected
   */
  bool connect_wifi(const char* ssid, const char* password = nullptr, bool first_time = false, bool wait_for_result = true);

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
  bool start_ap_server(uint16_t device_id, const char* password);

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
  const char* get_hostname() const;

  void update_active();
  bool was_active();

  /**
   * Register the WiFi station disconnect event handler, so that a dropped
   * link can be detected immediately instead of waiting for the next poll.
   */
  void register_events();

  /**
   * Returns true (once) if a STA disconnect event fired since the last call.
   */
  bool consume_disconnect_event();

  /**
   * Marks that a STA disconnect event fired. Called from the WiFi event callback.
   */
  void flag_disconnect_event();

 private:
  char _hostname[20];
  uint32_t _last_activity;
  bool _disconnect_event;
};

extern WiFiWrapper WiFiWrapper_i;

#endif //BGEIGIEZEN_WIFI_WRAPPER_H_
