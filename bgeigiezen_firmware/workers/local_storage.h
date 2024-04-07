#ifndef BGEIGIEZEN_ESP_CONFIG_H_
#define BGEIGIEZEN_ESP_CONFIG_H_

#include <Preferences.h>

#include <Handler.hpp>

#define CONFIG_VAL_MAX 32

/**
 * Configurations for the ESP32, stored in the flash memory
 */
class LocalStorage : public ProcessWorker<bool> {
 public:
  LocalStorage();
  virtual ~LocalStorage() = default;

  /**
   * Reset settings to default (defined in user_config)
   */
  void reset_defaults();

  // Getters and setters
  virtual uint16_t get_device_id() const final;
  virtual uint32_t get_fixed_device_id() const final;
  virtual const char* get_ap_password() const final;
  virtual uint16_t get_alarm_threshold() const final;
  virtual bool get_cpm_usvh() const final;
  virtual bool get_manual_logging() const final;
  virtual bool get_enable_journal() const final;
  virtual bool get_log_void() const final;
  virtual uint16_t get_screen_dim_timeout() const final;
  virtual uint16_t get_screen_off_timeout() const final;
  virtual bool get_animated_screensaver() const final;
  virtual const char* get_wifi_ssid() const final;
  virtual const char* get_wifi_password() const final;
  virtual const char* get_api_key() const final;
  virtual double get_fixed_longitude() const final;
  virtual double get_fixed_latitude() const final;
  virtual float get_fixed_range() const final;
  virtual double get_last_longitude() const final;
  virtual double get_last_latitude() const final;

  virtual void set_device_id(uint16_t device_id, bool force);
  virtual void set_ap_password(const char* ap_password, bool force);
  virtual void set_alarm_threshold(uint16_t alarm_threshold, bool force);
  virtual void set_cpm_usvh(bool cpm_usvh, bool force);
  virtual void set_manual_logging(bool manual_logging, bool force);
  virtual void set_enable_journal(bool enable_journal, bool force);
  virtual void set_log_void(bool log_void, bool force);
  virtual void set_screen_dim_timeout(uint16_t screen_dim_timeout, bool force);
  virtual void set_screen_off_timeout(uint16_t screen_off_timeout, bool force);
  virtual void set_animated_screensaver(bool animated_screensaver, bool force);
  virtual void set_wifi_ssid(const char* wifi_ssid, bool force);
  virtual void set_wifi_password(const char* wifi_password, bool force);
  virtual void set_api_key(const char* api_key, bool force);
  virtual void set_fixed_longitude(double fixed_longitude, bool force);
  virtual void set_fixed_latitude(double fixed_latitude, bool force);
  virtual void set_last_longitude(double last_longitude, bool force);
  virtual void set_last_latitude(double last_latitude, bool force);
  virtual void set_fixed_range(float fixed_range, bool force);

 protected:
  virtual bool clear();

  /**
   * Read all settings
   */
  bool activate(bool) override;
  int8_t produce_data(const worker_map_t& workers) override;
 private:
  Preferences _memory;

  // Device settings
  uint16_t _device_id;
  uint16_t _alarm_threshold;
  bool _cpm_usvh; // Main display CPM (true) or uSv/h (false) values
  bool _manual_logging;
  bool _enable_journal;
  bool _log_void; // include invalid lines (void gps/gm) in data logs
  uint16_t _screen_dim_timeout; // in seconds
  uint16_t _screen_off_timeout; // in seconds
  bool _animated_screensaver;
  char _ap_password[CONFIG_VAL_MAX];

  // Connection settings
  char _wifi_ssid[CONFIG_VAL_MAX];
  char _wifi_password[CONFIG_VAL_MAX];
  char _api_key[CONFIG_VAL_MAX];

  // Location settings
  double _fixed_longitude;
  double _fixed_latitude;
  float _fixed_range;

  double _last_longitude;
  double _last_latitude;
};

#endif //BGEIGIEZEN_ESP_CONFIG_H_
