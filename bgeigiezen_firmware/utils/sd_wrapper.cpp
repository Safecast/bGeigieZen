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
#define SD_CONFIG_FIELD_CLICK_SOUND_LEVEL "click_sound_level"
#define SD_CONFIG_FIELD_ALARM_THRESHOLD "alarm_threshold"
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
constexpr char sd_config_access_point_password_f[] = SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD"=%s";
constexpr char sd_config_click_sound_level_f[] = SD_CONFIG_FIELD_CLICK_SOUND_LEVEL"=%hhu";
constexpr char sd_config_alarm_threshold_f[] = SD_CONFIG_FIELD_ALARM_THRESHOLD"=%d";
constexpr char sd_config_wifi_ssid_f[] = SD_CONFIG_FIELD_WIFI_SSID"=%s";
constexpr char sd_config_wifi_password_f[] = SD_CONFIG_FIELD_WIFI_PASSWORD"=%s";
constexpr char sd_config_api_key_f[] = SD_CONFIG_FIELD_API_KEY"=%s";
constexpr char sd_config_fixed_latitude_f[] = SD_CONFIG_FIELD_FIXED_LATITUDE"=%lf";
constexpr char sd_config_fixed_longitude_f[] = SD_CONFIG_FIELD_FIXED_LONGITUDE"=%lf";
constexpr char sd_config_fixed_range_f[] = SD_CONFIG_FIELD_FIXED_RANGE"=%hu";


SDInterface::SDInterface() : _sd_ready(false), _last_read(0), _last_write(0), _last_try(0) {
}

bool SDInterface::ready() {
  if (!SD.exists(TEST_FILENAME)) {
    end();
  }
  return _sd_ready;
}

/**
 * We note that for the ESP32, the SD library is robust to multiple call to the begin() function
 * @return true if ready
 */
bool SDInterface::begin() {
  if (!_sd_ready && (_last_try == 0 || _last_try + 5000 < millis())) {
    _last_try = millis();
    if (SD.begin(SD_CS_PIN)) {
      bool zen_test = SD.exists(TEST_FILENAME);
      if (!zen_test) {
        auto new_file = SD.open(TEST_FILENAME, FILE_WRITE);
        if (new_file) {
          new_file.close();
        }
      }
      _sd_ready = SD.exists(TEST_FILENAME);
    }
  }
  return _sd_ready;
}

void SDInterface::end() {
  _sd_ready = false;
  SD.end();
}

bool SDInterface::log(const char* log_name, const uint8_t* data, size_t buff_size) {
  if (!_sd_ready) {
    return false;
  }
  if (!SD.exists(log_name)) {
    DEBUG_PRINTF("Unable to log to non-existent file \"%s\".\n", log_name);
    return false;
  }
  // open the setup file
  auto log_file = SD.open(log_name, FILE_WRITE);
  if (!log_file) {
    end();
    DEBUG_PRINTF("Unable to open log file \"%s\".\n", log_name);
    return false;
  }

  log_file.write(data, buff_size);

  _last_write = millis();
  return false;
}

SDInterface::SdStatus SDInterface::has_safezen_content(uint16_t device_id) {
  if (!_sd_ready) {
    return SdStatus::e_sd_config_status_not_ready;
  }

  if (!SD.exists(SETUP_FILENAME)) {
    return SdStatus::e_sd_config_status_no_config_file;
  }

  // open the setup file
  auto setup_file = SD.open(SETUP_FILENAME, FILE_READ);
  _last_read = millis();

  if (!setup_file) {
    return SdStatus::e_sd_config_status_config_no_content;
  }

  uint32_t file_device_id = 0;

  while (setup_file.available()) {
    String line = setup_file.readStringUntil('\n');
    if (line.startsWith(SD_CONFIG_FIELD_DEVICE_ID)) {
      sscanf(line.c_str(), sd_config_device_id_f, &file_device_id);
      break;
    }
  }

  // close the setup file
  setup_file.close();

  if (file_device_id == 0) {
    return SdStatus::e_sd_config_status_config_no_content;
  } else if (device_id && file_device_id != device_id) {
    return SdStatus::e_sd_config_status_config_different_id;
  } else {
    return SdStatus::e_sd_config_status_ok;
  }
}

bool SDInterface::read_safezen_file(LocalStorage& settings) {
  if (!_sd_ready) {
    return false;
  }
  File safecast_txt = SD.open(SETUP_FILENAME, FILE_READ);
  _last_read = millis();
  if (!safecast_txt) {
    return false;
  }

  char version[CONFIG_VAL_MAX] = "";

  String first_line = safecast_txt.readStringUntil('\n');
  if (first_line.startsWith(SD_CONFIG_FIELD_VERSION)) {
    sscanf(first_line.c_str(), sd_config_version_f, version);
  } else {
    // Expect latest version
    strcpy(version, VERSION_STRING);
  }

  if (strcmp(version, VERSION_STRING) == 0) {
    return read_safezen_file_latest(settings, safecast_txt);
  }

  return false;
}

bool SDInterface::read_safezen_file_latest(LocalStorage& settings, File& file) {
  // Device settings
  uint32_t device_id = 0;
  char access_point_password[CONFIG_VAL_MAX] = "";
  uint8_t click_sound_level = 0;
  uint32_t alarm_threshold = 0;

  // Connection settings
  char wifi_ssid[CONFIG_VAL_MAX] = "";
  char wifi_password[CONFIG_VAL_MAX] = "";
  char api_key[CONFIG_VAL_MAX] = "";

  // Location settings
  double fixed_latitude = 0;
  double fixed_longitude = 0;
  uint16_t fixed_range = 0;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() == 0) {
      continue;
    }
    else if (line.startsWith(SD_CONFIG_FIELD_DEVICE_ID)) {
      if (sscanf(line.c_str(), sd_config_device_id_f, &device_id)) {
        settings.set_device_id(device_id, true);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_API_KEY)) {
      if (device_id && sscanf(line.c_str(), sd_config_api_key_f, api_key)) {
        settings.set_api_key(api_key, true);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD)) {
      if (device_id && sscanf(line.c_str(), sd_config_access_point_password_f, access_point_password)) {
        settings.set_ap_password(access_point_password, true);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_WIFI_SSID)) {
      if (device_id && sscanf(line.c_str(), sd_config_wifi_ssid_f, wifi_ssid)) {
        settings.set_wifi_ssid(wifi_ssid, true);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_WIFI_PASSWORD)) {
      if (device_id && sscanf(line.c_str(), sd_config_wifi_password_f, wifi_password)) {
        settings.set_wifi_password(wifi_password, true);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_CLICK_SOUND_LEVEL)) {
      if (device_id && sscanf(line.c_str(), sd_config_click_sound_level_f, &click_sound_level)) {
        settings.set_click_sound_level(click_sound_level, true);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_ALARM_THRESHOLD)) {
      if (device_id && sscanf(line.c_str(), sd_config_alarm_threshold_f, &alarm_threshold)) {
        settings.set_alarm_threshold(alarm_threshold, true);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_FIXED_LATITUDE)) {
      if (device_id && sscanf(line.c_str(), sd_config_fixed_latitude_f, &fixed_latitude)) {
        settings.set_fixed_latitude(fixed_latitude, true);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_FIXED_LONGITUDE)) {
      if (device_id && sscanf(line.c_str(), sd_config_fixed_longitude_f, &fixed_longitude)) {
        settings.set_fixed_longitude(fixed_longitude, true);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_FIXED_RANGE)) {
      if (device_id && sscanf(line.c_str(), sd_config_fixed_range_f, &fixed_range)) {
        settings.set_fixed_range(fixed_range, true);
      }
    } else {
      DEBUG_PRINTF("Read ??? SAFEZEN line: %s \n", line.c_str());
    }
  }

  file.close();

  _last_read = millis();

  return !!device_id;
}

bool SDInterface::write_safezen_file(const LocalStorage& settings, bool full) {
  if (!_sd_ready) {
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

  safecast_txt.printf(sd_config_version_f, VERSION_STRING);
  safecast_txt.println();
  safecast_txt.printf(sd_config_device_id_f, settings.get_device_id());
  safecast_txt.println();
  safecast_txt.printf(sd_config_api_key_f, settings.get_api_key());
  safecast_txt.println();
  safecast_txt.printf(sd_config_access_point_password_f, settings.get_ap_password());
  safecast_txt.println();
  safecast_txt.printf(sd_config_wifi_ssid_f, settings.get_wifi_ssid());
  safecast_txt.println();
  safecast_txt.printf(sd_config_wifi_password_f, settings.get_wifi_password());
  safecast_txt.println();
  safecast_txt.printf(sd_config_click_sound_level_f, settings.get_click_sound_level());
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
    DEBUG_PRINTF("Finished writing SAFEZEN file:\n%s\n", written_safecast_txt.readString().c_str());
  }
  return true;
}
