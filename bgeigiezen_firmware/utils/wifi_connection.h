#ifndef BGEIGIEZEN_WIFI_WRAPPER_H_
#define BGEIGIEZEN_WIFI_WRAPPER_H_

#include <RBD_Timer.h>

class WiFiWrapper {
 public:
  WiFiWrapper();
  /**
   * Fire a connect attempt (WiFi.begin()/WiFi.reconnect()) and return
   * immediately — this NEVER blocks waiting for the result. Callers that
   * need to know the outcome must poll wifi_connected() / connecting() /
   * connect_timed_out() on later loop() ticks or render passes.
   * @param ssid
   * @param password
   * @return true only if WiFi was already connected before this call
   */
  bool connect_wifi(const char* ssid, const char* password = nullptr, bool first_time = false);

  /**
   * True while a connect attempt fired by connect_wifi() is still in
   * flight (not yet connected, not yet timed out).
   */
  bool connecting();

  /**
   * True if the most recent connect attempt has been running longer than
   * the connect timeout without succeeding.
   */
  bool connect_timed_out();

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
  bool _connect_attempt_active;
  RBD::Timer _connect_timer;
};

extern WiFiWrapper WiFiWrapper_i;

#endif //BGEIGIEZEN_WIFI_WRAPPER_H_
