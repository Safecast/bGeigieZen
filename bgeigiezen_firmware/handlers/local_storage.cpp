#include "local_storage.h"
#include "debugger.h"
#include "identifiers.h"
#include "controller.h"
#include "api_connector.h"
#include "workers/gps_connector.h"

const char* memory_name = "data";

// Keys for config
constexpr char const* key_device_id = "device_id";
constexpr char const* key_ap_password = "device_password";
constexpr char const* key_wifi_ssid = "wifi_ssid";
constexpr char const* key_wifi_password = "wifi_password";
constexpr char const* key_api_key = "api_key";
constexpr char const* key_alarm_threshold = "alarm_threshold";
constexpr char const* key_fixed_range = "fixed_range";
constexpr char const* key_fixed_longitude = "fixed_longitude";
constexpr char const* key_fixed_latitude = "fixed_latitude";
constexpr char const* key_last_longitude = "last_longitude";
constexpr char const* key_last_latitude = "last_latitude";

LocalStorage::LocalStorage() :
    ProcessWorker<bool>(),
    _memory(),
    _device_id(0),
    _ap_password(""),
    _alarm_threshold(0),
    _wifi_ssid(""),
    _wifi_password(""),
    _api_key(""),
    _fixed_longitude(0),
    _fixed_latitude(0),
    _fixed_range(0),
    _last_longitude(0),
    _last_latitude(0) {
}

void LocalStorage::reset_defaults() {
  if(clear()) {
    set_device_id(D_DEVICE_ID, true);
    set_ap_password(D_ACCESS_POINT_PASSWORD, true);
    set_alarm_threshold(100, true);
    set_wifi_ssid(D_WIFI_SSID, true);
    set_wifi_password(D_WIFI_PASSWORD, true);
    set_api_key(D_APIKEY, true);
    set_fixed_longitude(0, true);
    set_fixed_latitude(0, true);
    set_fixed_range(0, true);
    set_last_longitude(0, true);
    set_last_latitude(0, true);
  }
}

uint16_t LocalStorage::get_device_id() const {
  return _device_id;
}

const char* LocalStorage::get_ap_password() const {
  return _ap_password;
}

uint16_t LocalStorage::get_alarm_threshold() const {
  return _alarm_threshold;
}

const char* LocalStorage::get_wifi_ssid() const {
  return _wifi_ssid;
}

const char* LocalStorage::get_wifi_password() const {
  return _wifi_password;
}

const char* LocalStorage::get_api_key() const {
  return _api_key;
}

double LocalStorage::get_fixed_longitude() const {
  return _fixed_longitude;
}

double LocalStorage::get_fixed_latitude() const {
  return _fixed_latitude;
}

uint16_t LocalStorage::get_fixed_range() const {
  return _fixed_range;
}

double LocalStorage::get_last_longitude() const {
  return _last_longitude;
}

double LocalStorage::get_last_latitude() const {
  return _last_latitude;
}

void LocalStorage::set_device_id(uint16_t device_id, bool force) {
  if(force || (device_id != _device_id)) {
    if(_memory.begin(memory_name)) {
      _device_id = device_id;
      _memory.putUShort(key_device_id, _device_id);
      _memory.end();
    } else {
      DEBUG_PRINTLN("unable to save new value for device_id");
    }
  }
}

void LocalStorage::set_ap_password(const char* ap_password, bool force) {
  if(force || (ap_password != nullptr && strlen(ap_password) < CONFIG_VAL_MAX)) {
    if(_memory.begin(memory_name)) {
      strcpy(_ap_password, ap_password);
      _memory.putString(key_ap_password, _ap_password);
      _memory.end();
    } else {
      DEBUG_PRINTLN("unable to save new value for ap_password");
    }
  }
}

void LocalStorage::set_alarm_threshold(uint16_t alarm_threshold, bool force) {
  if(_memory.begin(memory_name)) {
    _alarm_threshold = alarm_threshold;
    _memory.putUInt(key_alarm_threshold, alarm_threshold);
    _memory.end();
  } else {
    DEBUG_PRINTLN("unable to save new value for ap_password");
  }
}

void LocalStorage::set_wifi_ssid(const char* wifi_ssid, bool force) {
  if(force || (wifi_ssid != nullptr && strlen(wifi_ssid) < CONFIG_VAL_MAX)) {
    if(_memory.begin(memory_name)) {
      strcpy(_wifi_ssid, wifi_ssid);
      _memory.putString(key_wifi_ssid, _wifi_ssid);
      _memory.end();
    } else {
      DEBUG_PRINTLN("unable to save new value for wifi_ssid");
    }
  }
}

void LocalStorage::set_wifi_password(const char* wifi_password, bool force) {
  if(force || (wifi_password != nullptr && strlen(wifi_password) < CONFIG_VAL_MAX)) {
    if(_memory.begin(memory_name)) {
      strcpy(_wifi_password, wifi_password);
      _memory.putString(key_wifi_password, _wifi_password);
      _memory.end();
    } else {
      DEBUG_PRINTLN("unable to save new value for wifi_password");
    }
  }
}

void LocalStorage::set_api_key(const char* api_key, bool force) {
  if(force || (api_key != nullptr && strlen(api_key) < CONFIG_VAL_MAX)) {
    if(_memory.begin(memory_name)) {
      strcpy(_api_key, api_key);
      _memory.putString(key_api_key, _api_key);
      _memory.end();
    } else {
      DEBUG_PRINTLN("unable to save new value for api_key");
    }
  }
}

void LocalStorage::set_fixed_longitude(double fixed_longtitude, bool force) {
  if(_memory.begin(memory_name)) {
    _fixed_longitude = fixed_longtitude;
    _memory.putDouble(key_fixed_longitude, fixed_longtitude);
    _memory.end();
  } else {
    DEBUG_PRINTLN("unable to save new value for key_fixed_longitude");
  }
}

void LocalStorage::set_fixed_latitude(double fixed_latitude, bool force) {
  if(_memory.begin(memory_name)) {
    _fixed_latitude = fixed_latitude;
    _memory.putDouble(key_fixed_latitude, fixed_latitude);
    _memory.end();
  } else {
    DEBUG_PRINTLN("unable to save new value for key_fixed_latitude");
  }
}

void LocalStorage::set_fixed_range(uint16_t fixed_range, bool force) {
  if(_memory.begin(memory_name)) {
    _fixed_range = fixed_range;
    _memory.putUInt(key_fixed_range, fixed_range);
    _memory.end();
  } else {
    DEBUG_PRINTLN("unable to save new value for key_fixed_range");
  }
}

void LocalStorage::set_last_longitude(double last_longitude, bool force) {
  if(_memory.begin(memory_name)) {
    _last_longitude = last_longitude;
    _memory.putDouble(key_last_longitude, last_longitude);
    _memory.end();
  } else {
    DEBUG_PRINTLN("unable to save new value for key_last_longitude");
  }
}

void LocalStorage::set_last_latitude(double last_latitude, bool force) {
  if(_memory.begin(memory_name)) {
    _last_latitude = last_latitude;
    _memory.putDouble(key_last_latitude, last_latitude);
    _memory.end();
  } else {
    DEBUG_PRINTLN("unable to save new value for key_last_latitude");
  }
}

bool LocalStorage::clear() {
  if(_memory.begin(memory_name)) {
    _memory.clear();
    _memory.end();
    return true;
  }
  return false;
}

bool LocalStorage::activate(bool) {
  _memory.begin(memory_name, true);
  _device_id = _memory.getUShort(key_device_id, D_DEVICE_ID);
  if(_memory.getString(key_ap_password, _ap_password, CONFIG_VAL_MAX) == 0) {
    strcpy(_ap_password, D_ACCESS_POINT_PASSWORD);
  }
  if(_memory.getString(key_wifi_ssid, _wifi_ssid, CONFIG_VAL_MAX) == 0) {
    strcpy(_wifi_ssid, D_WIFI_SSID);
  }
  if(_memory.getString(key_wifi_password, _wifi_password, CONFIG_VAL_MAX) == 0) {
    strcpy(_wifi_password, D_WIFI_PASSWORD);
  }
  if(_memory.getString(key_api_key, _api_key, CONFIG_VAL_MAX) == 0) {
    strcpy(_api_key, D_APIKEY);
  }
  _fixed_longitude = _memory.getDouble(key_fixed_longitude, 0);
  _fixed_latitude = _memory.getDouble(key_fixed_latitude, 0);
  _fixed_range = _memory.getUInt(key_fixed_range, 0);
  _last_longitude = _memory.getDouble(key_last_longitude, 0);
  _last_latitude = _memory.getDouble(key_last_latitude, 0);
  _memory.end();
  return true;
}

int8_t LocalStorage::produce_data(const worker_map_t& workers) {
  // Get reading data to store
  const auto& gps = (GpsConnector*) workers.at(k_worker_gps_connector);
  if(gps->is_fresh() && gps->get_data().location_valid) {
    set_last_latitude(gps->get_data().latitude, false);
    set_last_longitude(gps->get_data().longitude, false);
    return Worker::e_worker_data_read;
  }
  return Worker::e_worker_idle;
}
