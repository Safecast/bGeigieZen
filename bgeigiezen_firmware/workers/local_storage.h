#ifndef BGEIGIEZEN_ESP_CONFIG_H_
#define BGEIGIEZEN_ESP_CONFIG_H_

#include <Preferences.h>

#include <Handler.hpp>

#define CONFIG_VAL_MAX 32
#define CONFIG_LONG_VAL_MAX 64

/**
 * Configurations for the ESP32, stored in the flash memory
 */
class LocalStorage : public ProcessWorker<bool> {
 public:
  enum OperationalMode {
    e_operational_mode_drive = 0,
    e_operational_mode_survey,
    e_operational_mode_fixed,
    e_operational_mode_satellite,
<<<<<<< HEAD
    e_operational_mode_flight,
=======
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
  };

  LocalStorage();
  virtual ~LocalStorage() = default;

  /**
   * Reset settings to default (defined in user_config)
   */
  void reset_defaults();

  // Getters and setters
  virtual uint16_t get_device_id() const final;
  virtual uint32_t get_fixed_device_id() const final;
  virtual const char* get_user_name() const final;
  virtual const char* get_ap_password() const final;
  virtual uint16_t get_alert_threshold() const final;
  virtual bool get_cpm_usvh() const final;
  virtual bool get_manual_logging() const final;
  virtual bool get_enable_journal() const final;
  virtual bool get_log_void() const final;
  virtual uint16_t get_screen_dim_timeout() const final;
  virtual uint16_t get_screen_off_timeout() const final;
  virtual bool get_animated_screensaver() const final;
<<<<<<< HEAD
  virtual bool get_error_alert_sound() const final;
  // Screen dim brightness percentage (0-100)
  virtual uint8_t get_dim_brightness() const final;
  // Global audio volume percentage (0-100)
  virtual uint8_t get_audio_volume() const final;
  // Primary WiFi profile
  virtual const char* get_wifi_ssid() const final;
  virtual const char* get_wifi_password() const final;
  // Secondary WiFi profile
  virtual const char* get_wifi_ssid2() const final;
  virtual const char* get_wifi_password2() const final;

  // Active profile helper (1 or 2)
  virtual uint8_t get_wifi_profile_active() const final;

  // Active profile convenience getters
  virtual const char* get_active_wifi_ssid() const final;
  virtual const char* get_active_wifi_password() const final;

=======
  virtual const char* get_wifi_ssid() const final;
  virtual const char* get_wifi_password() const final;
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
  virtual const char* get_api_key() const final;
  virtual double get_fixed_longitude() const final;
  virtual double get_fixed_latitude() const final;
  virtual float get_fixed_range() const final;
  virtual uint16_t get_dop_max() const final;
  virtual double get_last_longitude() const final;
  virtual double get_last_latitude() const final;
  virtual OperationalMode get_last_mode() const final;

  virtual void set_device_id(uint16_t device_id, bool force);
  virtual void set_user_name(const char* user_name, bool force);
  virtual void set_ap_password(const char* ap_password, bool force);
  virtual void set_alert_threshold(uint16_t alert_threshold, bool force);
  virtual void set_cpm_usvh(bool cpm_usvh, bool force);
  virtual void set_manual_logging(bool manual_logging, bool force);
  virtual void set_enable_journal(bool enable_journal, bool force);
  virtual void set_log_void(bool log_void, bool force);
  virtual void set_screen_dim_timeout(uint16_t screen_dim_timeout, bool force);
  virtual void set_screen_off_timeout(uint16_t screen_off_timeout, bool force);
  virtual void set_animated_screensaver(bool animated_screensaver, bool force);
<<<<<<< HEAD
  virtual void set_error_alert_sound(bool error_alert_sound, bool force);
  virtual void set_dim_brightness(uint8_t dim_brightness, bool force);
  virtual void set_audio_volume(uint8_t audio_volume, bool force);
  virtual void set_wifi_ssid(const char* wifi_ssid, bool force);
  virtual void set_wifi_password(const char* wifi_password, bool force);
  virtual void set_wifi_ssid2(const char* wifi_ssid, bool force);
  virtual void set_wifi_password2(const char* wifi_password, bool force);
  virtual void set_wifi_profile_active(uint8_t profile, bool force);
=======
  virtual void set_wifi_ssid(const char* wifi_ssid, bool force);
  virtual void set_wifi_password(const char* wifi_password, bool force);
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
  virtual void set_api_key(const char* api_key, bool force);
  virtual void set_fixed_longitude(double fixed_longitude, bool force);
  virtual void set_fixed_latitude(double fixed_latitude, bool force);
  virtual void set_fixed_range(float fixed_range, bool force);
  virtual void set_dop_max(uint16_t dop_max, bool force);
  virtual void set_last_longitude(double last_longitude, bool force);
  virtual void set_last_latitude(double last_latitude, bool force);
  virtual void set_last_mode(OperationalMode last_mode, bool force);
<<<<<<< HEAD
  virtual void reset_dose_rate() final;
  virtual void save_accumulated_dose(float dose) final;
  virtual float get_accumulated_dose() const final;
  
=======
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416

 protected:
  virtual bool clear();

  /**
   * Read all settings
   */
  bool activate(bool) override;
  int8_t produce_data(const worker_map_t& workers) override;
 private:
<<<<<<< HEAD
  mutable Preferences _memory;
=======
  Preferences _memory;
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416

  // Device settings
  uint16_t _device_id;
  char _user_name[CONFIG_VAL_MAX];
  uint16_t _alert_threshold;
  bool _cpm_usvh; // Main display CPM (true) or uSv/h (false) values
  bool _manual_logging;
  bool _enable_journal;
  bool _log_void; // include invalid lines (void gps/gm) in data logs
  uint16_t _screen_dim_timeout; // in seconds
  uint16_t _screen_off_timeout; // in seconds
<<<<<<< HEAD
  uint8_t _dim_brightness; // percentage (0-100) used when DIM or screensaver is active
  uint8_t _audio_volume; // percentage (0-100) global audio volume for clicks/alerts
  bool _animated_screensaver;
  bool _error_alert_sound;
  char _ap_password[CONFIG_VAL_MAX];

  // Connection settings
  // WiFi profiles
  char _wifi_ssid[CONFIG_VAL_MAX];
  char _wifi_password[CONFIG_LONG_VAL_MAX];
  char _wifi_ssid2[CONFIG_VAL_MAX];
  char _wifi_password2[CONFIG_LONG_VAL_MAX];
  uint8_t _wifi_profile_active; // 1 or 2

=======
  bool _animated_screensaver;
  char _ap_password[CONFIG_VAL_MAX];

  // Connection settings
  char _wifi_ssid[CONFIG_VAL_MAX];
  char _wifi_password[CONFIG_LONG_VAL_MAX];
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
  char _api_key[CONFIG_VAL_MAX];

  // Location settings
  double _fixed_longitude;
  double _fixed_latitude;
  float _fixed_range;
  uint16_t _dop_max; // value as DOP * 100


  // internal tracking
  OperationalMode _last_mode;
  double _last_longitude;
  double _last_latitude;
<<<<<<< HEAD
  
=======
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416

};

#endif //BGEIGIEZEN_ESP_CONFIG_H_
