#ifndef BGEIGIEZEN_ESP_CONFIG_H_
#define BGEIGIEZEN_ESP_CONFIG_H_

#include <Preferences.h>

#include <Handler.hpp>

#define CONFIG_VAL_MAX 32

/**
 * Configurations for the ESP32, stored in the flash memory
 */
class LocalStorage : public Handler {
 public:
  LocalStorage();
  virtual ~LocalStorage() = default;

  /**
   * Reset settings to default (defined in user_config)
   */
  void reset_defaults();

  // Getters and setters
  virtual uint16_t get_device_id() const final;
  virtual const char* get_ap_password() const final;
  virtual const char* get_wifi_ssid() const final;
  virtual const char* get_wifi_password() const final;
  virtual const char* get_api_key() const final;
  virtual uint8_t get_send_frequency() const final;
  virtual double get_fixed_longitude() const final;
  virtual double get_fixed_latitude() const final;
  virtual double get_last_longitude() const final;
  virtual double get_last_latitude() const final;

  virtual void set_device_id(uint16_t device_id, bool force);
  virtual void set_ap_password(const char* ap_password, bool force);
  virtual void set_wifi_ssid(const char* wifi_ssid, bool force);
  virtual void set_wifi_password(const char* wifi_password, bool force);
  virtual void set_api_key(const char* api_key, bool force);
  virtual void set_send_frequency(uint8_t send_frequency, bool force);
  virtual void set_fixed_longitude(double fixed_longitude, bool force);
  virtual void set_fixed_latitude(double fixed_latitude, bool force);
  virtual void set_last_longitude(double last_longitude, bool force);
  virtual void set_last_latitude(double last_latitude, bool force);

 protected:
  virtual bool clear();

  /**
   * Read all settings
   */
  bool activate(bool) override;
  int8_t handle_produced_work(const worker_map_t& workers) override;
 private:
  Preferences _memory;

  // Device settings
  uint16_t _device_id;
  char _ap_password[CONFIG_VAL_MAX];

  // Connection settings
  char _wifi_ssid[CONFIG_VAL_MAX];
  char _wifi_password[CONFIG_VAL_MAX];
  char _api_key[CONFIG_VAL_MAX];
  uint8_t _send_frequency;

  // Location settings
  double _fixed_longitude;
  double _fixed_latitude;

  double _last_longitude;
  double _last_latitude;
};

#endif //BGEIGIEZEN_ESP_CONFIG_H_
