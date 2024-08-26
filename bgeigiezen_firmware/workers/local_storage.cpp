#include "local_storage.h"
#include "identifiers.h"
#include "controller.h"
#include "workers/gps_connector.h"

const char* memory_name = "data";

// Keys for config
constexpr char const* key_device_id = "device_id";
constexpr char const* key_ap_password = "device_password";
constexpr char const* key_wifi_ssid = "wifi_ssid";
constexpr char const* key_wifi_password = "wifi_password";
constexpr char const* key_api_key = "api_key";
constexpr char const* key_alert_threshold = "alarm_threshold";
constexpr char const* key_cpm_usvh = "cpm_usvh";
constexpr char const* key_manual_logging = "manual_logging";
constexpr char const* key_enable_journal = "enable_journal";
constexpr char const* key_log_void = "log_void";
constexpr char const* key_dop_max = "dop_max";
constexpr char const* key_screen_dim_timeout = "dim_timeout";
constexpr char const* key_screen_off_timeout = "off_timeout";
constexpr char const* key_animated_screensaver = "ani_screensaver";
constexpr char const* key_fixed_range = "fixed_range";
constexpr char const* key_fixed_longitude = "fixed_longitude";
constexpr char const* key_fixed_latitude = "fixed_latitude";
constexpr char const* key_last_longitude = "last_longitude";
constexpr char const* key_last_latitude = "last_latitude";
constexpr char const* key_last_mode = "last_mode";

LocalStorage::LocalStorage() :
    ProcessWorker<bool>(),
    _memory(),
    _device_id(0),
    _ap_password(""),
    _alert_threshold(0),
    _cpm_usvh(false),
    _manual_logging(false),
    _enable_journal(true),
    _log_void(false),
    _screen_dim_timeout(60),
    _screen_off_timeout(600),
    _animated_screensaver(true),
    _wifi_ssid(""),
    _wifi_password(""),
    _api_key(""),
    _fixed_longitude(0),
    _fixed_latitude(0),
    _fixed_range(0.5),
    _dop_max(0),
    _last_longitude(0),
    _last_latitude(0),
    _last_mode(e_operational_mode_drive) {
}

void LocalStorage::reset_defaults() {
  if(clear()) {
    set_device_id(D_DEVICE_ID, true);
    set_ap_password(D_AP_PASSWORD, true);
    set_alert_threshold(D_ALARM_THRESHOLD, true);
    set_cpm_usvh(D_CPM_USVH, true);
    set_manual_logging(D_MANUAL_LOGGING, true);
    set_enable_journal(D_ENABLE_JOURNAL, true);
    set_log_void(D_LOG_VOID, true);
    set_screen_dim_timeout(D_SCREEN_DIM_TIMEOUT, true);
    set_screen_off_timeout(D_SCREEN_OFF_TIMEOUT, true);
    set_animated_screensaver(D_ANIMATED_SCREENSAVER, true);
    set_wifi_ssid(D_WIFI_SSID, true);
    set_wifi_password(D_WIFI_PASSWORD, true);
    set_api_key(D_API_KEY, true);
    set_fixed_longitude(D_FIXED_LONGITUDE, true);
    set_fixed_latitude(D_FIXED_LATITUDE, true);
    set_fixed_range(D_FIXED_RANGE, true);
    set_dop_max(D_DOP_MAX, true);
    set_last_longitude(D_LAST_LONGITUDE, true);
    set_last_latitude(D_LAST_LATITUDE, true);
    set_last_mode(e_operational_mode_drive, true);
    M5_LOGD("Local Storage: Set defaults for all settings");
  }
}

uint16_t LocalStorage::get_device_id() const {
  return _device_id;
}

uint32_t LocalStorage::get_fixed_device_id() const {
  return _device_id >= 5000 && _device_id < 6000 ? 60000 + _device_id : 0;
}

const char* LocalStorage::get_ap_password() const {
  return _ap_password;
}

uint16_t LocalStorage::get_alert_threshold() const {
  return _alert_threshold;
}

bool LocalStorage::get_cpm_usvh() const {
  return _cpm_usvh;
}

bool LocalStorage::get_manual_logging() const {
  return _manual_logging;
}

bool LocalStorage::get_enable_journal() const {
  return _enable_journal;
}

bool LocalStorage::get_log_void() const {
  return _log_void;
}

uint16_t LocalStorage::get_dop_max() const {
  return _dop_max;
}

uint16_t LocalStorage::get_screen_dim_timeout() const {
  return _screen_dim_timeout;
}

uint16_t LocalStorage::get_screen_off_timeout() const {
  return _screen_off_timeout;
}

bool LocalStorage::get_animated_screensaver() const {
  return _animated_screensaver;
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

float LocalStorage::get_fixed_range() const {
  return _fixed_range;
}

double LocalStorage::get_last_longitude() const {
  return _last_longitude;
}

double LocalStorage::get_last_latitude() const {
  return _last_latitude;
}

LocalStorage::OperationalMode LocalStorage::get_last_mode() const {
  return _last_mode;
}

void LocalStorage::set_device_id(uint16_t device_id, bool force) {
  if((force || (device_id != _device_id)) && device_id > 5000 && device_id < 6000) {
    if(_memory.begin(memory_name)) {
      _device_id = device_id;
      _memory.putUShort(key_device_id, _device_id);
      _memory.end();
    } else {
      M5_LOGD("unable to save new value for device_id");
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
      M5_LOGD("unable to save new value for ap_password");
    }
  }
}

void LocalStorage::set_alert_threshold(uint16_t alert_threshold, bool force) {
  if(_memory.begin(memory_name)) {
    _alert_threshold = alert_threshold;
    _memory.putUInt(key_alert_threshold, alert_threshold);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for ap_password");
  }
}

void LocalStorage::set_cpm_usvh(bool cpm_usvh, bool force) {
  if(_memory.begin(memory_name)) {
    _cpm_usvh = cpm_usvh;
    _memory.putBool(key_cpm_usvh, cpm_usvh);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for cpm_usvh");
  }
}

void LocalStorage::set_manual_logging(bool manual_logging, bool force) {
  if(_memory.begin(memory_name)) {
    _manual_logging = manual_logging;
    _memory.putBool(key_manual_logging, manual_logging);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for manual_logging");
  }
}

void LocalStorage::set_enable_journal(bool enable_journal, bool force) {
  if(_memory.begin(memory_name)) {
    _enable_journal = enable_journal;
    _memory.putBool(key_enable_journal, enable_journal);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for enable_journal");
  }
}

void LocalStorage::set_log_void(bool log_void, bool force) {
  if(_memory.begin(memory_name)) {
    _log_void = log_void;
    _memory.putBool(key_log_void, log_void);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for log_void");
  }
}

void LocalStorage::set_dop_max(uint16_t dop_max, bool force) {
  if(_memory.begin(memory_name)) {
    _dop_max = dop_max;
    _memory.putUInt(key_dop_max, dop_max);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for dop_max");
  }
}

void LocalStorage::set_screen_dim_timeout(uint16_t screen_dim_timeout, bool force) {
  if(_memory.begin(memory_name)) {
    _screen_dim_timeout = screen_dim_timeout;
    _memory.putUInt(key_screen_dim_timeout, screen_dim_timeout);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for screen_dim_timeout");
  }
}

void LocalStorage::set_screen_off_timeout(uint16_t screen_off_timeout, bool force) {
  if(_memory.begin(memory_name)) {
    _screen_off_timeout = screen_off_timeout;
    _memory.putUInt(key_screen_off_timeout, screen_off_timeout);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for screen_off_timeout");
  }
}

void LocalStorage::set_animated_screensaver(bool animated_screensaver, bool force) {
  if(_memory.begin(memory_name)) {
    _animated_screensaver = animated_screensaver;
    _memory.putBool(key_animated_screensaver, animated_screensaver);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for animated_screensaver");
  }
}

void LocalStorage::set_wifi_ssid(const char* wifi_ssid, bool force) {
  if(force || (wifi_ssid != nullptr && strlen(wifi_ssid) < CONFIG_VAL_MAX)) {
    if(_memory.begin(memory_name)) {
      strcpy(_wifi_ssid, wifi_ssid);
      _memory.putString(key_wifi_ssid, _wifi_ssid);
      _memory.end();
    } else {
      M5_LOGD("unable to save new value for wifi_ssid");
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
      M5_LOGD("unable to save new value for wifi_password");
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
      M5_LOGD("unable to save new value for api_key");
    }
  }
}

void LocalStorage::set_fixed_longitude(double fixed_longitude, bool force) {
  if(_memory.begin(memory_name)) {
    _fixed_longitude = fixed_longitude;
    _memory.putDouble(key_fixed_longitude, fixed_longitude);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for key_fixed_longitude");
  }
}

void LocalStorage::set_fixed_latitude(double fixed_latitude, bool force) {
  if(_memory.begin(memory_name)) {
    _fixed_latitude = fixed_latitude;
    _memory.putDouble(key_fixed_latitude, fixed_latitude);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for key_fixed_latitude");
  }
}

void LocalStorage::set_fixed_range(float fixed_range, bool force) {
  if(_memory.begin(memory_name)) {
    _fixed_range = fixed_range;
    _memory.putFloat(key_fixed_range, fixed_range);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for key_fixed_range");
  }
}

void LocalStorage::set_last_longitude(double last_longitude, bool force) {
  if(_memory.begin(memory_name)) {
    _last_longitude = last_longitude;
    _memory.putDouble(key_last_longitude, last_longitude);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for key_last_longitude");
  }
}

void LocalStorage::set_last_latitude(double last_latitude, bool force) {
  if(_memory.begin(memory_name)) {
    _last_latitude = last_latitude;
    _memory.putDouble(key_last_latitude, last_latitude);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for key_last_latitude");
  }
}

void LocalStorage::set_last_mode(LocalStorage::OperationalMode last_mode, bool force){
  if(_memory.begin(memory_name)) {
    _last_mode = last_mode;
    _memory.putUShort(key_last_mode, last_mode);
    _memory.end();
  } else {
    M5_LOGD("unable to save new value for key_last_mode");
  }
}

bool LocalStorage::clear() {
  if(_memory.begin(memory_name)) {
    _memory.clear();
    _memory.end();
    M5_LOGD("Local Storage: Cleared local memory...");
    return true;
  }
  M5_LOGD("Local Storage: Unable to clear local memory...");
  return false;
}

bool LocalStorage::activate(bool) {
  _memory.begin(memory_name, true);
  _device_id = _memory.getUShort(key_device_id, D_DEVICE_ID);
  _alert_threshold = _memory.getUInt(key_alert_threshold, D_ALARM_THRESHOLD);
  _cpm_usvh = _memory.getBool(key_cpm_usvh, D_CPM_USVH);
  _manual_logging = _memory.getBool(key_manual_logging, D_MANUAL_LOGGING);
  _enable_journal = _memory.getBool(key_enable_journal, D_ENABLE_JOURNAL);
  _log_void = _memory.getBool(key_log_void, D_LOG_VOID);
  _screen_dim_timeout = _memory.getUInt(key_screen_dim_timeout, D_SCREEN_DIM_TIMEOUT);
  _screen_off_timeout = _memory.getUInt(key_screen_off_timeout, D_SCREEN_OFF_TIMEOUT);
  _animated_screensaver = _memory.getBool(key_animated_screensaver, D_ANIMATED_SCREENSAVER);
  if(_memory.getString(key_ap_password, _ap_password, CONFIG_VAL_MAX) == 0) {
    strcpy(_ap_password, D_AP_PASSWORD);
  }
  if(_memory.getString(key_wifi_ssid, _wifi_ssid, CONFIG_VAL_MAX) == 0) {
    strcpy(_wifi_ssid, D_WIFI_SSID);
  }
  if(_memory.getString(key_wifi_password, _wifi_password, CONFIG_VAL_MAX) == 0) {
    strcpy(_wifi_password, D_WIFI_PASSWORD);
  }
  if(_memory.getString(key_api_key, _api_key, CONFIG_VAL_MAX) == 0) {
    strcpy(_api_key, D_API_KEY);
  }
  _fixed_longitude = _memory.getDouble(key_fixed_longitude, D_FIXED_LONGITUDE);
  _fixed_latitude = _memory.getDouble(key_fixed_latitude, D_FIXED_LATITUDE);
  _fixed_range = _memory.getFloat(key_fixed_range, D_FIXED_RANGE);
  _dop_max = _memory.getUInt(key_dop_max, D_DOP_MAX);
  _last_longitude = _memory.getDouble(key_last_longitude, D_LAST_LONGITUDE);
  _last_latitude = _memory.getDouble(key_last_latitude, D_LAST_LATITUDE);
  _last_mode = static_cast<OperationalMode>(_memory.getUShort(key_last_mode, e_operational_mode_drive));
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
