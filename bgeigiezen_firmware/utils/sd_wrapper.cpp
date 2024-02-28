#include "sd_wrapper.h"

#include "user_config.h"
#include "debugger.h"

#define SD_CONFIG_FIELD_VERSION "version"
#define SD_CONFIG_FIELD_DEVICE_ID "device_id"
#define SD_CONFIG_FIELD_USH_DIVIDER "ush_divider"
#define SD_CONFIG_FIELD_CPMN "cpmn"
#define SD_CONFIG_FIELD_BQM_FACTOR "bqm_factor"
#define SD_CONFIG_FIELD_BQMN "bqmn"
#define SD_CONFIG_FIELD_COUNTRY_CODE "country_code"
#define SD_CONFIG_FIELD_GT "gt"
#define SD_CONFIG_FIELD_GM "gm"
#define SD_CONFIG_FIELD_SENSOR_TYPE "sensor_type"
#define SD_CONFIG_FIELD_SENSOR_SHIELD "sensor_shield"
#define SD_CONFIG_FIELD_SENSOR_MODE "sensor_mode"
#define SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD "access_point_password"
#define SD_CONFIG_FIELD_ALARM_THRESHOLD "alarm_threshold"
#define SD_CONFIG_FIELD_MANUAL_LOGGING "manual_logging"
#define SD_CONFIG_FIELD_WIFI_SSID "wifi_ssid"
#define SD_CONFIG_FIELD_WIFI_PASSWORD "wifi_password"
#define SD_CONFIG_FIELD_API_KEY "api_key"
#define SD_CONFIG_FIELD_FIXED_LATITUDE "fixed_latitude"
#define SD_CONFIG_FIELD_FIXED_LONGITUDE "fixed_longitude"
#define SD_CONFIG_FIELD_FIXED_RANGE "fixed_range"

constexpr char sd_config_version_f[] = SD_CONFIG_FIELD_VERSION"=%s";
constexpr char sd_config_device_id_f[] = SD_CONFIG_FIELD_DEVICE_ID"=%u";
constexpr char sd_config_ush_divider_f[] = SD_CONFIG_FIELD_USH_DIVIDER"=%lf";
constexpr char sd_config_cpmn_f[] = SD_CONFIG_FIELD_CPMN"=%s";
constexpr char sd_config_bqm_factor_f[] = SD_CONFIG_FIELD_BQM_FACTOR"=%lf";
constexpr char sd_config_bqmn_f[] = SD_CONFIG_FIELD_BQMN"=%s";
constexpr char sd_config_country_code_f[] = SD_CONFIG_FIELD_COUNTRY_CODE"=%s";
constexpr char sd_config_gt_f[] = SD_CONFIG_FIELD_GT"=%d";
constexpr char sd_config_gm_f[] = SD_CONFIG_FIELD_GM"=%d";
constexpr char sd_config_sensor_type_f[] = SD_CONFIG_FIELD_SENSOR_TYPE"=%d";
constexpr char sd_config_sensor_shield_f[] = SD_CONFIG_FIELD_SENSOR_SHIELD"=%d";
constexpr char sd_config_sensor_mode_f[] = SD_CONFIG_FIELD_SENSOR_MODE"=%d";
constexpr char sd_config_access_point_password_f[] = SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD"=%[^\t\r\n]";
constexpr char sd_config_access_point_password_write_f[] = SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD"=%s";
constexpr char sd_config_alarm_threshold_f[] = SD_CONFIG_FIELD_ALARM_THRESHOLD"=%d";
constexpr char sd_config_manual_logging_f[] = SD_CONFIG_FIELD_MANUAL_LOGGING"=%hhu";
constexpr char sd_config_wifi_ssid_f[] = SD_CONFIG_FIELD_WIFI_SSID"=%[^\t\r\n]";
constexpr char sd_config_wifi_ssid_write_f[] = SD_CONFIG_FIELD_WIFI_SSID"=%s";
constexpr char sd_config_wifi_password_f[] = SD_CONFIG_FIELD_WIFI_PASSWORD"=%[^\t\r\n]";
constexpr char sd_config_wifi_password_write_f[] = SD_CONFIG_FIELD_WIFI_PASSWORD"=%s";
constexpr char sd_config_api_key_f[] = SD_CONFIG_FIELD_API_KEY"=%s";
constexpr char sd_config_fixed_latitude_f[] = SD_CONFIG_FIELD_FIXED_LATITUDE"=%lf";
constexpr char sd_config_fixed_longitude_f[] = SD_CONFIG_FIELD_FIXED_LONGITUDE"=%lf";
constexpr char sd_config_fixed_range_f[] = SD_CONFIG_FIELD_FIXED_RANGE"=%f";


SDInterface::SDInterface() : _status(SdStatus::e_sd_config_status_not_ready), _last_read(0), _last_write(0), _last_try(0) {
}

bool SDInterface::ready() {
  if (_status != e_sd_config_status_not_ready && !SD.exists(TEST_FILENAME)) {
    end();
  }
  return _status != e_sd_config_status_not_ready;
}

bool SDInterface::can_write_logs() const {
  return _status == e_sd_config_status_ok;
}

SDInterface::SdStatus SDInterface::status() const {
  return _status;
}

/**
 * We note that for the ESP32, the SD library is robust to multiple call to the begin() function
 * @return true if ready
 */
bool SDInterface::begin() {
  static bool _first_try = true;
  if (!ready() && (_first_try || (millis() - _last_try > 5000))) {
    _first_try = false;
    _last_try = millis();
    if (SD.begin(SD_CS_PIN)) {
      bool zen_test = SD.exists(TEST_FILENAME);
      if (!zen_test) {
        auto new_file = SD.open(TEST_FILENAME, FILE_WRITE);
        if (new_file) {
          new_file.close();
        }
      }
      _status = SD.exists(TEST_FILENAME) ? e_sd_config_status_config_no_content : e_sd_config_status_not_ready;
    }
  }
  return ready();
}

bool SDInterface::just_wrote() const {
  return _last_write && (_last_write + 500) > millis();
}

void SDInterface::end() {
  _status = e_sd_config_status_not_ready;
  SD.end();
}

bool SDInterface::log_println(const char* log_name, const char* data) {
  if (!ready()) {
    return false;
  }
  if (!SD.exists(log_name)) {
    DEBUG_PRINTF("Unable to log to non-existent file \"%s\".\n", log_name);
    return false;
  }
  // open the setup file in append mode to add lines
  auto log_file = SD.open(log_name, FILE_APPEND);
  if (!log_file) {
    end();
    DEBUG_PRINTF("Unable to open log file \"%s\".\n", log_name);
    return false;
  }

  log_file.println(data);

//  DEBUG_PRINTF("Logged '%s' to '%s'.\n", data, log_name);

  log_file.close();
  _last_write = millis();
  return true;
}

SDInterface::SdStatus SDInterface::has_safezen_content(uint16_t device_id) {
  if (!ready()) {
    _status = e_sd_config_status_not_ready;
    return _status;
  }

  if (!SD.exists(SETUP_FILENAME)) {
    return _status = e_sd_config_status_no_config_file;
  }

  // open the setup file
  auto setup_file = SD.open(SETUP_FILENAME, FILE_READ);
  _last_read = millis();

  if (!setup_file) {
    return _status = e_sd_config_status_config_no_content;
  }

  _device_id = 0;

  while (setup_file.available()) {
    String line = setup_file.readStringUntil('\n');
    if (line.startsWith(SD_CONFIG_FIELD_DEVICE_ID)) {
      sscanf(line.c_str(), sd_config_device_id_f, &_device_id);
      break;
    }
  }

  // close the setup file
  setup_file.close();

  if (_device_id == 0) {
    _status = e_sd_config_status_config_no_content;
  } else if (device_id && _device_id != device_id) {
    _status = e_sd_config_status_config_different_id;
  } else {
    _status = e_sd_config_status_ok;
  }
  return _status;
}

bool SDInterface::read_safezen_file_to_settings(LocalStorage& settings) {
  DEBUG_PRINTLN("read_safezen_file_to_settings");
  if (!ready()) {
    return false;
  }

  File safecast_txt = SD.open(SETUP_FILENAME, FILE_READ);
  if (!safecast_txt) {
    return false;
  }
  _last_read = millis();

  char version[CONFIG_VAL_MAX] = "";

  String first_line = safecast_txt.readStringUntil('\n');
  if (first_line.startsWith(SD_CONFIG_FIELD_VERSION)) {
    sscanf(first_line.c_str(), sd_config_version_f, version);
  } else {
    // No version line read, expect latest version
    strcpy(version, VERSION_NUMBER);
    // Reopen settings, because first line was (probably) some configuration
    safecast_txt.close();
    safecast_txt = SD.open(SETUP_FILENAME, FILE_READ);
    if (!safecast_txt) {
      return false;
    }
  }

  // Add earlier / non-compatible versions here too
  if (strcmp(version, VERSION_NUMBER) == 0) {
    return read_safezen_file_latest(settings, safecast_txt);
  }

  // try with latest, at this point it should at least get the id
  return read_safezen_file_latest(settings, safecast_txt);
}

bool SDInterface::read_safezen_file_latest(LocalStorage& settings, File& file) {
  // Device settings
  char access_point_password[CONFIG_VAL_MAX] = "";
  uint32_t alarm_threshold = 0;
  uint8_t manual_logging = false;

  // Connection settings
  char wifi_ssid[CONFIG_VAL_MAX] = "";
  char wifi_password[CONFIG_VAL_MAX] = "";
  char api_key[CONFIG_VAL_MAX] = "";

  // Location settings
  double fixed_latitude = 0;
  double fixed_longitude = 0;
  float fixed_range = 0;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() == 0) {
      continue;
    }
    else if (line.startsWith(SD_CONFIG_FIELD_DEVICE_ID)) {
      if (sscanf(line.c_str(), sd_config_device_id_f, &_device_id)) {
        settings.set_device_id(_device_id, true);
        DEBUG_PRINTF("Loaded from SD: device_id=%d\n", _device_id);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_API_KEY)) {
      if (_device_id && line.length() - strlen(SD_CONFIG_FIELD_API_KEY) < CONFIG_VAL_MAX && sscanf(line.c_str(), sd_config_api_key_f, api_key)) {
        settings.set_api_key(api_key, true);
        DEBUG_PRINTF("Loaded from SD: api_key=%s\n", api_key);
      } else {
        DEBUG_PRINTLN("Unable to load api_key");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD)) {
      if (_device_id && line.length() - strlen(SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD) < CONFIG_VAL_MAX && sscanf(line.c_str(), sd_config_access_point_password_f, access_point_password)) {
        settings.set_ap_password(access_point_password, true);
        DEBUG_PRINTF("Loaded from SD: access_point_password=%s\n", access_point_password);
      } else {
        DEBUG_PRINTLN("Unable to load access_point_password");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_WIFI_SSID)) {
      if (_device_id && line.length() - strlen(SD_CONFIG_FIELD_WIFI_SSID) < CONFIG_VAL_MAX && sscanf(line.c_str(), sd_config_wifi_ssid_f, wifi_ssid)) {
        settings.set_wifi_ssid(wifi_ssid, true);
        DEBUG_PRINTF("Loaded from SD: wifi_ssid=%s\n", wifi_ssid);
      } else {
        DEBUG_PRINTLN("Unable to load wifi_ssid");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_WIFI_PASSWORD)) {
      if (_device_id && line.length() - strlen(SD_CONFIG_FIELD_WIFI_PASSWORD) < CONFIG_VAL_MAX && sscanf(line.c_str(), sd_config_wifi_password_f, wifi_password)) {
        settings.set_wifi_password(wifi_password, true);
        DEBUG_PRINTF("Loaded from SD: wifi_password=%s\n", wifi_password);
      } else {
        DEBUG_PRINTLN("Unable to load wifi_password");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_ALARM_THRESHOLD)) {
      if (_device_id && sscanf(line.c_str(), sd_config_alarm_threshold_f, &alarm_threshold)) {
        settings.set_alarm_threshold(alarm_threshold, true);
        DEBUG_PRINTF("Loaded from SD: alarm_threshold=%d\n", alarm_threshold);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_MANUAL_LOGGING)) {
      if (_device_id && sscanf(line.c_str(), sd_config_manual_logging_f, &manual_logging)) {
        settings.set_manual_logging(!!manual_logging, true);
        DEBUG_PRINTF("Loaded from SD: manual_logging=%d\n", !!manual_logging);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_FIXED_LATITUDE)) {
      if (_device_id && sscanf(line.c_str(), sd_config_fixed_latitude_f, &fixed_latitude)) {
        settings.set_fixed_latitude(fixed_latitude, true);
        DEBUG_PRINTF("Loaded from SD: fixed_latitude=%0.6f\n", fixed_latitude);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_FIXED_LONGITUDE)) {
      if (_device_id && sscanf(line.c_str(), sd_config_fixed_longitude_f, &fixed_longitude)) {
        settings.set_fixed_longitude(fixed_longitude, true);
        DEBUG_PRINTF("Loaded from SD: fixed_longitude=%0.6f\n", fixed_longitude);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_FIXED_RANGE)) {
      if (_device_id && sscanf(line.c_str(), sd_config_fixed_range_f, &fixed_range)) {
//        settings.set_fixed_range(fixed_range, true);
//        DEBUG_PRINTF("Loaded from SD: fixed_range=%0.1f\n", fixed_longitude);
        DEBUG_PRINTF("Currently disabled setting: fixed_range=%0.1f\n", fixed_longitude);
      }
    } else {
      DEBUG_PRINTF("Read (currently) unsupported SAFEZEN line: %s \n", line.c_str());
    }
  }

  file.close();

  _last_read = millis();

  if (_device_id == settings.get_device_id()) {
    DEBUG_PRINTLN("Successfully loaded settings from SD config into memory");
    _status = e_sd_config_status_ok;
  } else {
    _status = e_sd_config_status_no_config_file;
  }
  return !!_device_id;
}

bool SDInterface::write_safezen_file_from_settings(const LocalStorage& settings, bool full) {
  if (!ready()) {
    return false;
  }
  if (!settings.get_device_id()) {
    return false;
  }

  if (SD.exists(SETUP_FILENAME)) {
    SD.remove(SETUP_FILENAME);
  }

  File safecast_txt = SD.open(SETUP_FILENAME, FILE_WRITE);
  _last_write = millis();
  if (!safecast_txt) {
    end();
    return false;
  }

  DEBUG_PRINTLN("Writing SAFEZEN file");

  safecast_txt.printf(sd_config_version_f, VERSION_NUMBER);
  safecast_txt.println();
  safecast_txt.printf(sd_config_device_id_f, settings.get_device_id());
  safecast_txt.println();
  safecast_txt.printf(sd_config_api_key_f, settings.get_api_key());
  safecast_txt.println();
  safecast_txt.printf(sd_config_access_point_password_write_f, settings.get_ap_password());
  safecast_txt.println();
  safecast_txt.printf(sd_config_wifi_ssid_write_f, settings.get_wifi_ssid());
  safecast_txt.println();
  safecast_txt.printf(sd_config_wifi_password_write_f, settings.get_wifi_password());
  safecast_txt.println();
  safecast_txt.printf(sd_config_alarm_threshold_f, settings.get_alarm_threshold());
  safecast_txt.println();
  safecast_txt.printf(sd_config_fixed_latitude_f, settings.get_fixed_latitude());
  safecast_txt.println();
  safecast_txt.printf(sd_config_fixed_longitude_f, settings.get_fixed_longitude());
  safecast_txt.println();
  safecast_txt.printf(sd_config_fixed_range_f, settings.get_fixed_range());
  safecast_txt.println();

  if (full) {
//      safecast_txt.printf(sd_config_ush_divider_f, settings.get_ush_divider());
//      safecast_txt.println();
//      safecast_txt.printf(sd_config_cpmn_f, settings.get_cpmn());
//      safecast_txt.println();
//      safecast_txt.printf(sd_config_bqm_factor_f, settings.get_bqm_factor());
//      safecast_txt.println();
//      safecast_txt.printf(sd_config_bqmn_f, settings.get_bqmn());
//      safecast_txt.println();
//      safecast_txt.printf(sd_config_country_code_f, settings.get_country_code());
//      safecast_txt.println();
//      safecast_txt.printf(sd_config_gt_f, settings.get_gt());
//      safecast_txt.println();
//      safecast_txt.printf(sd_config_gm_f, settings.get_gm());
//      safecast_txt.println();
//      safecast_txt.printf(sd_config_sensor_type_f, settings.get_sensor_type());
//      safecast_txt.println();
//      safecast_txt.printf(sd_config_sensor_shield_f, settings.get_sensor_shield());
//      safecast_txt.println();
//      safecast_txt.printf(sd_config_sensor_mode_f, settings.get_sensor_mode());
//      safecast_txt.println();
  }

  safecast_txt.close();

  File written_safecast_txt = SD.open(SETUP_FILENAME, FILE_READ);
  if (safecast_txt) {
    _status = e_sd_config_status_ok;
    DEBUG_PRINTF("Finished writing SAFEZEN file:\n%s\n", written_safecast_txt.readString().c_str());
  }

  return true;
}

bool SDInterface::setup_log(const char* dir, const char* log_name_output, bool clear_on_exist) {
  if (SD.exists(log_name_output)) {
    if (clear_on_exist) {
      SD.remove(log_name_output);
    } else {
      return false;
    }
  } else {
    SD.mkdir(dir);
  }
  File safecast_txt = SD.open(log_name_output, FILE_WRITE);
  _last_write = millis();
  if (!safecast_txt) {
    end();
    return false;
  }
  safecast_txt.printf("");
  safecast_txt.close();
  return true;
}

bool SDInterface::rename_log(const char* old_log_name, const char* new_log_name) {
  if (!ready()) {
    return false;
  }

  if (!SD.exists(old_log_name)) {
    return false;
  }
  return SD.rename(old_log_name, new_log_name);
}

bool SDInterface::delete_log(const char* log_name) {
  if (!ready()) {
    return false;
  }

  return SD.remove(log_name);
}
