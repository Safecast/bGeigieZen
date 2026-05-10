#include "sd_wrapper.h"
#include "user_config.h"
#include "workers/sound_manager.h"

extern SoundManager sound_manager;

#define SD_CONFIG_FIELD_VERSION "version"
#define SD_CONFIG_FIELD_DEVICE_ID "device_id"
#define SD_CONFIG_FIELD_USER_NAME "user_name"
#define SD_CONFIG_FIELD_COUNTRY_CODE "country_code"
#define SD_CONFIG_FIELD_USH_DIVIDER "ush_divider"
#define SD_CONFIG_FIELD_CPMN "cpmn"
#define SD_CONFIG_FIELD_BQM_FACTOR "bqm_factor"
#define SD_CONFIG_FIELD_BQMN "bqmn"
#define SD_CONFIG_FIELD_GT "gt"
#define SD_CONFIG_FIELD_GM "gm"
#define SD_CONFIG_FIELD_SENSOR_TYPE "sensor_type"
#define SD_CONFIG_FIELD_SENSOR_SHIELD "sensor_shield"
#define SD_CONFIG_FIELD_SENSOR_MODE "sensor_mode"
#define SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD "access_point_password"
#define SD_CONFIG_FIELD_ALARM_THRESHOLD "alert_threshold"
#define SD_CONFIG_FIELD_CPM_USVH "display_cpm"
#define SD_CONFIG_FIELD_MANUAL_LOGGING "manual_logging"
#define SD_CONFIG_FIELD_ENABLE_JOURNAL "enable_journal"
#define SD_CONFIG_FIELD_LOG_VOID "log_void"
#define SD_CONFIG_FIELD_SCREEN_DIM_TIMEOUT "screen_dim_timeout"
#define SD_CONFIG_FIELD_SCREEN_OFF_TIMEOUT "screen_off_timeout"
#define SD_CONFIG_FIELD_ANIMATED_SCREENSAVER "animated_screensaver"
#define SD_CONFIG_FIELD_ERROR_ALERT_SOUND "error_alert_sound"
#define SD_CONFIG_FIELD_SOUND_ENABLED "sound_enabled"
#define SD_CONFIG_FIELD_DIM_BRIGHTNESS "dim_brightness"
#define SD_CONFIG_FIELD_AUDIO_VOLUME "audio_volume"
#define SD_CONFIG_FIELD_WIFI_SSID "wifi_ssid"
#define SD_CONFIG_FIELD_WIFI_PASSWORD "wifi_password"
#define SD_CONFIG_FIELD_WIFI_SSID2 "wifi_ssid2"
#define SD_CONFIG_FIELD_WIFI_PASSWORD2 "wifi_password2"
#define SD_CONFIG_FIELD_WIFI_PROFILE "wifi_profile"
#define SD_CONFIG_FIELD_API_KEY "api_key"
#define SD_CONFIG_FIELD_API_LOGFILE_ENDPOINT_2 "api_logfile_endpoint_2"
#define SD_CONFIG_FIELD_API_LOGFILE_DEST "api_logfile_dest"
#define SD_CONFIG_FIELD_FIXED_LATITUDE "fixed_latitude"
#define SD_CONFIG_FIELD_FIXED_LONGITUDE "fixed_longitude"
#define SD_CONFIG_FIELD_FIXED_RANGE "fixed_range"
#define SD_CONFIG_FIELD_DOP_MAX "dop_max"

constexpr char sd_config_version_f[] = SD_CONFIG_FIELD_VERSION"=%s";
constexpr char sd_config_device_id_f[] = SD_CONFIG_FIELD_DEVICE_ID"=%u";
constexpr char sd_config_user_name_f[] = SD_CONFIG_FIELD_USER_NAME"=%[^\t\r\n]";
constexpr char sd_config_user_name_write_f[] = SD_CONFIG_FIELD_USER_NAME"=%s";
constexpr char sd_config_country_code_f[] = SD_CONFIG_FIELD_COUNTRY_CODE"=%s";
constexpr char sd_config_ush_divider_f[] = SD_CONFIG_FIELD_USH_DIVIDER"=%lf";
constexpr char sd_config_cpmn_f[] = SD_CONFIG_FIELD_CPMN"=%s";
constexpr char sd_config_bqm_factor_f[] = SD_CONFIG_FIELD_BQM_FACTOR"=%lf";
constexpr char sd_config_bqmn_f[] = SD_CONFIG_FIELD_BQMN"=%s";
constexpr char sd_config_gt_f[] = SD_CONFIG_FIELD_GT"=%d";
constexpr char sd_config_gm_f[] = SD_CONFIG_FIELD_GM"=%d";
constexpr char sd_config_sensor_type_f[] = SD_CONFIG_FIELD_SENSOR_TYPE"=%d";
constexpr char sd_config_sensor_shield_f[] = SD_CONFIG_FIELD_SENSOR_SHIELD"=%d";
constexpr char sd_config_sensor_mode_f[] = SD_CONFIG_FIELD_SENSOR_MODE"=%d";
constexpr char sd_config_access_point_password_f[] = SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD"=%[^\t\r\n]";
constexpr char sd_config_access_point_password_write_f[] = SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD"=%s";
constexpr char sd_config_alert_threshold_f[] = SD_CONFIG_FIELD_ALARM_THRESHOLD"=%d";
constexpr char sd_config_cpm_usvh_f[] = SD_CONFIG_FIELD_CPM_USVH"=%hhu";
constexpr char sd_config_manual_logging_f[] = SD_CONFIG_FIELD_MANUAL_LOGGING"=%hhu";
constexpr char sd_config_enable_journal_f[] = SD_CONFIG_FIELD_ENABLE_JOURNAL"=%hhu";
constexpr char sd_config_log_void_f[] = SD_CONFIG_FIELD_LOG_VOID"=%hhu";
constexpr char sd_config_screen_dim_timeout_f[] = SD_CONFIG_FIELD_SCREEN_DIM_TIMEOUT"=%du";
constexpr char sd_config_screen_off_timeout_f[] = SD_CONFIG_FIELD_SCREEN_OFF_TIMEOUT"=%du";
constexpr char sd_config_animated_screensaver_f[] = SD_CONFIG_FIELD_ANIMATED_SCREENSAVER"=%hhu";
constexpr char sd_config_error_alert_sound_f[] = SD_CONFIG_FIELD_ERROR_ALERT_SOUND"=%hhu";
constexpr char sd_config_sound_enabled_f[] = SD_CONFIG_FIELD_SOUND_ENABLED"=%hhu";
constexpr char sd_config_dim_brightness_f[] = SD_CONFIG_FIELD_DIM_BRIGHTNESS"=%hhu";
constexpr char sd_config_audio_volume_f[] = SD_CONFIG_FIELD_AUDIO_VOLUME"=%hhu";
constexpr char sd_config_wifi_ssid_f[] = SD_CONFIG_FIELD_WIFI_SSID"=%[^\t\r\n]";
constexpr char sd_config_wifi_ssid_write_f[] = SD_CONFIG_FIELD_WIFI_SSID"=%s";
constexpr char sd_config_wifi_password_f[] = SD_CONFIG_FIELD_WIFI_PASSWORD"=%[^\t\r\n]";
constexpr char sd_config_wifi_password_write_f[] = SD_CONFIG_FIELD_WIFI_PASSWORD"=%s";
constexpr char sd_config_wifi_ssid2_f[] = SD_CONFIG_FIELD_WIFI_SSID2"=%[^\t\r\n]";
constexpr char sd_config_wifi_ssid2_write_f[] = SD_CONFIG_FIELD_WIFI_SSID2"=%s";
constexpr char sd_config_wifi_password2_f[] = SD_CONFIG_FIELD_WIFI_PASSWORD2"=%[^\t\r\n]";
constexpr char sd_config_wifi_password2_write_f[] = SD_CONFIG_FIELD_WIFI_PASSWORD2"=%s";
constexpr char sd_config_wifi_profile_f[] = SD_CONFIG_FIELD_WIFI_PROFILE"=%hhu";
constexpr char sd_config_api_key_f[] = SD_CONFIG_FIELD_API_KEY"=%s";
constexpr char sd_config_api_logfile_endpoint_2_f[] = SD_CONFIG_FIELD_API_LOGFILE_ENDPOINT_2"=%[^\t\r\n]";
constexpr char sd_config_api_logfile_endpoint_2_write_f[] = SD_CONFIG_FIELD_API_LOGFILE_ENDPOINT_2"=%s";
constexpr char sd_config_api_logfile_dest_f[] = SD_CONFIG_FIELD_API_LOGFILE_DEST"=%hhu";
constexpr char sd_config_fixed_latitude_f[] = SD_CONFIG_FIELD_FIXED_LATITUDE"=%lf";
constexpr char sd_config_fixed_longitude_f[] = SD_CONFIG_FIELD_FIXED_LONGITUDE"=%lf";
constexpr char sd_config_fixed_range_f[] = SD_CONFIG_FIELD_FIXED_RANGE"=%f";
constexpr char sd_config_dop_max_f[] = SD_CONFIG_FIELD_DOP_MAX "=%hu";


SDInterface::SDInterface() : _status(SdStatus::e_sd_config_status_not_ready), _busy(false), _last_read(0), _last_write(0), _last_try(0) {
}

bool SDInterface::ready() {
  if (_busy) {
    return false;
  }
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

bool SDInterface::clear_all_logs() {
  if (!ready()) {
    return false;
  }
  delete_directory(JOURNAL_LOG_DIRECTORY);
  delete_directory(DRIVE_LOG_DIRECTORY);
  delete_directory(SURVEY_LOG_DIRECTORY);
  return true;
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
    // Attempt to auto-create the missing file and its parent directory
    String path = String(log_name);
    int last_slash = path.lastIndexOf('/');
    if (last_slash > 0) {
      String dir = path.substring(0, last_slash);
      if (!SD.exists(dir.c_str())) {
        M5_LOGD("Creating directory '%s' for missing log file", dir.c_str());
        SD.mkdir(dir.c_str());
      }
    }

    // Create the file
    File create_file = SD.open(log_name, FILE_WRITE);
    if (!create_file) {
      end();
      M5_LOGD("Unable to create log file \"%s\".", log_name);
      return false;
    }
    create_file.close();

    // If still doesn't exist, give up
    if (!SD.exists(log_name)) {
      M5_LOGD("Unable to log to non-existent file \"%s\" after creation attempt.", log_name);
      return false;
    }
  }
  // open the setup file in append mode to add lines
  auto log_file = SD.open(log_name, FILE_APPEND);
  if (!log_file) {
    end();
    M5_LOGD("Unable to open log file \"%s\".", log_name);
    return false;
  }

  log_file.println(data);

//  M5_LOGD("Logged '%s' to '%s'.", data, log_name);

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
  M5_LOGD("read_safezen_file_to_settings");
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
  uint32_t alert_threshold = 0;
  uint8_t cpm_usvh = false;
  uint8_t manual_logging = false;
  uint8_t enable_journal = true;
  uint8_t log_void = true;  // Always set to true to include invalid lines
  uint32_t screen_dim_timeout = 0;
  uint32_t screen_off_timeout = 0;
  uint8_t animated_screensaver = true;
  uint8_t error_alert_sound = true;
  uint8_t dim_brightness = D_DIM_BRIGHTNESS;
  uint8_t audio_volume = D_AUDIO_VOLUME;
  uint8_t sound_enabled = 1;

  // Connection settings
  char wifi_ssid[CONFIG_VAL_MAX] = "";
  char wifi_password[CONFIG_VAL_MAX] = "";
  char wifi_ssid2[CONFIG_VAL_MAX] = "";
  char wifi_password2[CONFIG_LONG_VAL_MAX] = "";
  uint8_t wifi_profile_active = 1;
  char api_key[CONFIG_VAL_MAX] = "";
  char api_logfile_endpoint_2[CONFIG_LONG_VAL_MAX] = "";
  uint8_t api_logfile_dest = 0;
  char user_name[CONFIG_VAL_MAX] = "";

  // Location settings
  double fixed_latitude = 0;
  double fixed_longitude = 0;
  float fixed_range = 0;
  uint16_t max_dop = 0;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() == 0) {
      continue;
    }
    else if (line.startsWith(SD_CONFIG_FIELD_DEVICE_ID)) {
      if (sscanf(line.c_str(), sd_config_device_id_f, &_device_id)) {
        settings.set_device_id(_device_id, true);
        M5_LOGD("Loaded from SD: device_id=%d", _device_id);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_USER_NAME)) {
      if (sscanf(line.c_str(), sd_config_user_name_f, user_name)) {
        settings.set_user_name(user_name, true);
        M5_LOGD("Loaded from SD: user_name=%d", user_name);
      }
      else {
        M5_LOGD("Unable to load user_name");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_API_KEY)) {
      if (_device_id && line.length() - strlen(SD_CONFIG_FIELD_API_KEY) < CONFIG_VAL_MAX && sscanf(line.c_str(), sd_config_api_key_f, api_key)) {
        settings.set_api_key(api_key, true);
        M5_LOGD("Loaded from SD: api_key=%s", api_key);
      } else {
        M5_LOGD("Unable to load api_key");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_API_LOGFILE_ENDPOINT_2)) {
      if (_device_id && line.length() - strlen(SD_CONFIG_FIELD_API_LOGFILE_ENDPOINT_2) < CONFIG_LONG_VAL_MAX && sscanf(line.c_str(), sd_config_api_logfile_endpoint_2_f, api_logfile_endpoint_2)) {
        settings.set_api_logfile_endpoint_2(api_logfile_endpoint_2, true);
        M5_LOGD("Loaded from SD: api_logfile_endpoint_2=%s", api_logfile_endpoint_2);
      } else {
        M5_LOGD("Unable to load api_logfile_endpoint_2");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_API_LOGFILE_DEST)) {
      if (_device_id && sscanf(line.c_str(), sd_config_api_logfile_dest_f, &api_logfile_dest)) {
        settings.set_api_logfile_dest(api_logfile_dest, true);
        M5_LOGD("Loaded from SD: api_logfile_dest=%u", api_logfile_dest);
      } else {
        M5_LOGD("Unable to load api_logfile_dest");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD)) {
      if (_device_id && line.length() - strlen(SD_CONFIG_FIELD_ACCESS_POINT_PASSWORD) < CONFIG_VAL_MAX && sscanf(line.c_str(), sd_config_access_point_password_f, access_point_password)) {
        settings.set_ap_password(access_point_password, true);
        M5_LOGD("Loaded from SD: access_point_password=%s", access_point_password);
      } else {
        M5_LOGD("Unable to load access_point_password");
      }
    }
    // Check wifi_ssid2 and wifi_password2 BEFORE wifi_ssid and wifi_password
    // because "wifi_ssid2" starts with "wifi_ssid" and would match incorrectly
    else if (line.startsWith(SD_CONFIG_FIELD_WIFI_SSID2)) {
      if (_device_id && line.length() - strlen(SD_CONFIG_FIELD_WIFI_SSID2) < CONFIG_VAL_MAX && sscanf(line.c_str(), sd_config_wifi_ssid2_f, wifi_ssid2)) {
        settings.set_wifi_ssid2(wifi_ssid2, true);
        M5_LOGD("Loaded from SD: wifi_ssid2=%s", wifi_ssid2);
      } else {
        M5_LOGD("Unable to load wifi_ssid2");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_WIFI_PASSWORD2)) {
      if (_device_id && line.length() - strlen(SD_CONFIG_FIELD_WIFI_PASSWORD2) < CONFIG_LONG_VAL_MAX && sscanf(line.c_str(), sd_config_wifi_password2_f, wifi_password2)) {
        settings.set_wifi_password2(wifi_password2, true);
        M5_LOGD("Loaded from SD: wifi_password2=%s", wifi_password2);
      } else {
        M5_LOGD("Unable to load wifi_password2");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_WIFI_SSID)) {
      if (_device_id && line.length() - strlen(SD_CONFIG_FIELD_WIFI_SSID) < CONFIG_VAL_MAX && sscanf(line.c_str(), sd_config_wifi_ssid_f, wifi_ssid)) {
        settings.set_wifi_ssid(wifi_ssid, true);
        M5_LOGD("Loaded from SD: wifi_ssid=%s", wifi_ssid);
      } else {
        M5_LOGD("Unable to load wifi_ssid");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_WIFI_PASSWORD)) {
      if (_device_id && line.length() - strlen(SD_CONFIG_FIELD_WIFI_PASSWORD) < CONFIG_VAL_MAX && sscanf(line.c_str(), sd_config_wifi_password_f, wifi_password)) {
        settings.set_wifi_password(wifi_password, true);
        M5_LOGD("Loaded from SD: wifi_password=%s", wifi_password);
      } else {
        M5_LOGD("Unable to load wifi_password");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_WIFI_PROFILE)) {
      if (_device_id && sscanf(line.c_str(), sd_config_wifi_profile_f, &wifi_profile_active)) {
        settings.set_wifi_profile_active(wifi_profile_active, true);
        M5_LOGD("Loaded from SD: wifi_profile_active=%u", wifi_profile_active);
      } else {
        M5_LOGD("Unable to load wifi_profile");
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_ALARM_THRESHOLD)) {
      if (_device_id && sscanf(line.c_str(), sd_config_alert_threshold_f, &alert_threshold)) {
        settings.set_alert_threshold(alert_threshold, true);
        M5_LOGD("Loaded from SD: alert_threshold=%d", alert_threshold);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_CPM_USVH)) {
      if (_device_id && sscanf(line.c_str(), sd_config_cpm_usvh_f, &cpm_usvh)) {
        settings.set_cpm_usvh(!!cpm_usvh, true);
        M5_LOGD("Loaded from SD: display_cpm=%d", !!cpm_usvh);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_MANUAL_LOGGING)) {
      if (_device_id && sscanf(line.c_str(), sd_config_manual_logging_f, &manual_logging)) {
        settings.set_manual_logging(!!manual_logging, true);
        M5_LOGD("Loaded from SD: manual_logging=%d", !!manual_logging);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_ENABLE_JOURNAL)) {
      if (_device_id && sscanf(line.c_str(), sd_config_enable_journal_f, &enable_journal)) {
        settings.set_enable_journal(!!enable_journal, true);
        M5_LOGD("Loaded from SD: enable_journal=%d", !!enable_journal);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_LOG_VOID)) {
      if (_device_id && sscanf(line.c_str(), sd_config_log_void_f, &log_void)) {
        settings.set_log_void(!!log_void, true);
        M5_LOGD("Loaded from SD: log_void=%d", !!log_void);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_SCREEN_DIM_TIMEOUT)) {
      if (_device_id && sscanf(line.c_str(), sd_config_screen_dim_timeout_f, &screen_dim_timeout)) {
        settings.set_screen_dim_timeout(screen_dim_timeout, true);
        M5_LOGD("Loaded from SD: manual_logging=%d", !!manual_logging);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_SCREEN_OFF_TIMEOUT)) {
      if (_device_id && sscanf(line.c_str(), sd_config_screen_off_timeout_f, &screen_off_timeout)) {
        settings.set_screen_off_timeout(screen_off_timeout, true);
        M5_LOGD("Loaded from SD: manual_logging=%d", !!manual_logging);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_ANIMATED_SCREENSAVER)) {
      if (_device_id && sscanf(line.c_str(), sd_config_animated_screensaver_f, &animated_screensaver)) {
        settings.set_animated_screensaver(!!animated_screensaver, true);
        M5_LOGD("Loaded from SD: animated_screensaver=%d", !!animated_screensaver);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_DIM_BRIGHTNESS)) {
      if (_device_id && sscanf(line.c_str(), sd_config_dim_brightness_f, &dim_brightness)) {
        if (dim_brightness > 100) dim_brightness = 100;
        settings.set_dim_brightness(dim_brightness, true);
        M5_LOGD("Loaded from SD: dim_brightness=%u", dim_brightness);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_AUDIO_VOLUME)) {
      if (_device_id && sscanf(line.c_str(), sd_config_audio_volume_f, &audio_volume)) {
        if (audio_volume > 100) audio_volume = 100;
        settings.set_audio_volume(audio_volume, true);
        M5_LOGD("Loaded from SD: audio_volume=%u", audio_volume);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_ERROR_ALERT_SOUND)) {
      if (_device_id && sscanf(line.c_str(), sd_config_error_alert_sound_f, &error_alert_sound)) {
        settings.set_error_alert_sound(!!error_alert_sound, true);
        M5_LOGD("Loaded from SD: error_alert_sound=%d", !!error_alert_sound);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_SOUND_ENABLED)) {
      if (_device_id && sscanf(line.c_str(), sd_config_sound_enabled_f, &sound_enabled)) {
        sound_manager.setSoundEnabled(!!sound_enabled);
        M5_LOGD("Loaded from SD: sound_enabled=%d", !!sound_enabled);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_FIXED_LATITUDE)) {
      if (_device_id && sscanf(line.c_str(), sd_config_fixed_latitude_f, &fixed_latitude)) {
        settings.set_fixed_latitude(fixed_latitude, true);
        M5_LOGD("Loaded from SD: fixed_latitude=%0.6f", fixed_latitude);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_FIXED_LONGITUDE)) {
      if (_device_id && sscanf(line.c_str(), sd_config_fixed_longitude_f, &fixed_longitude)) {
        settings.set_fixed_longitude(fixed_longitude, true);
        M5_LOGD("Loaded from SD: fixed_longitude=%0.6f", fixed_longitude);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_FIXED_RANGE)) {
      if (_device_id && sscanf(line.c_str(), sd_config_fixed_range_f, &fixed_range)) {
        settings.set_fixed_range(fixed_range, true);
        M5_LOGD("Loaded from SD: fixed_range=%0.1f", fixed_longitude);
      }
    }
    else if (line.startsWith(SD_CONFIG_FIELD_DOP_MAX)) {
      if (_device_id && sscanf(line.c_str(), sd_config_dop_max_f, &max_dop)) {
        settings.set_dop_max(max_dop, true);
        M5_LOGD("Loaded from SD: max_dop=%u", max_dop);
      }
    } else {
      M5_LOGD("Skipped (currently) unsupported SAFEZEN line: %s ", line.c_str());
    }
  }

  file.close();

  _last_read = millis();

  if (_device_id == settings.get_device_id()) {
    M5_LOGD("Successfully loaded settings from SD config into memory");
    _status = e_sd_config_status_ok;
  } else {
    _status = e_sd_config_status_no_config_file;
  }
  return !!_device_id;
}

bool SDInterface::write_safezen_file_from_settings(const LocalStorage& settings, bool just_id) {
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
  if (!safecast_txt) {
    end();
    return false;
  }

  _last_write = millis();

  M5_LOGD("Writing SAFEZEN file");

  if (just_id) {
    safecast_txt.printf(sd_config_device_id_f, settings.get_device_id());
    safecast_txt.println();
  } else {
    safecast_txt.printf(sd_config_version_f, VERSION_NUMBER);
    safecast_txt.println();
    safecast_txt.printf(sd_config_device_id_f, settings.get_device_id());
    safecast_txt.println();
    safecast_txt.printf(sd_config_user_name_write_f, settings.get_user_name());
    safecast_txt.println();
    safecast_txt.printf(sd_config_api_key_f, settings.get_api_key());
    safecast_txt.println();
    safecast_txt.printf(sd_config_api_logfile_endpoint_2_write_f, settings.get_api_logfile_endpoint_2());
    safecast_txt.println();
    safecast_txt.printf(sd_config_api_logfile_dest_f, settings.get_api_logfile_dest());
    safecast_txt.println();
    safecast_txt.printf(sd_config_access_point_password_write_f, settings.get_ap_password());
    safecast_txt.println();
    safecast_txt.printf(sd_config_wifi_ssid_write_f, settings.get_wifi_ssid());
    safecast_txt.println();
    safecast_txt.printf(sd_config_wifi_password_write_f, settings.get_wifi_password());
    safecast_txt.println();
    safecast_txt.printf(sd_config_wifi_ssid2_write_f, settings.get_wifi_ssid2());
    safecast_txt.println();
    safecast_txt.printf(sd_config_wifi_password2_write_f, settings.get_wifi_password2());
    safecast_txt.println();
    safecast_txt.printf(sd_config_wifi_profile_f, settings.get_wifi_profile_active());
    safecast_txt.println();
    safecast_txt.printf(sd_config_alert_threshold_f, settings.get_alert_threshold());
    safecast_txt.println();
    safecast_txt.printf(sd_config_cpm_usvh_f, settings.get_cpm_usvh());
    safecast_txt.println();
    safecast_txt.printf(sd_config_manual_logging_f, settings.get_manual_logging());
    safecast_txt.println();
    safecast_txt.printf(sd_config_enable_journal_f, settings.get_enable_journal());
    safecast_txt.println();
    safecast_txt.printf(sd_config_log_void_f, settings.get_log_void());
    safecast_txt.println();
    safecast_txt.printf(sd_config_dim_brightness_f, settings.get_dim_brightness());
    safecast_txt.println();
    safecast_txt.printf(sd_config_audio_volume_f, settings.get_audio_volume());
    safecast_txt.println();
    safecast_txt.printf(sd_config_error_alert_sound_f, settings.get_error_alert_sound());
    safecast_txt.println();
    safecast_txt.printf(sd_config_sound_enabled_f, sound_manager.isSoundEnabled() ? 1 : 0);
    safecast_txt.println();
    safecast_txt.printf(sd_config_fixed_latitude_f, settings.get_fixed_latitude());
    safecast_txt.println();
    safecast_txt.printf(sd_config_fixed_longitude_f, settings.get_fixed_longitude());
    safecast_txt.println();
    safecast_txt.printf(sd_config_fixed_range_f, settings.get_fixed_range());
    safecast_txt.println();
    safecast_txt.printf(sd_config_dop_max_f, settings.get_dop_max());
    safecast_txt.println();
  }

//  if (full) {
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
//  }

  safecast_txt.close();

  File written_safecast_txt = SD.open(SETUP_FILENAME, FILE_READ);
  if (written_safecast_txt) {
    _status = e_sd_config_status_ok;
    M5_LOGD("Finished writing SAFEZEN file:\n%s", written_safecast_txt.readString().c_str());
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
    M5_LOGE("Cannot rename: SD card not ready");
    return false;
  }

  if (!SD.exists(old_log_name)) {
    M5_LOGE("Cannot rename: source file '%s' does not exist", old_log_name);
    return false;
  }
  
  // Check if target already exists. If so, assume rename was successful already.
  if (SD.exists(new_log_name)) {
    M5_LOGW("Target file '%s' already exists, assuming rename is complete.", new_log_name);
    // If the original file still exists, something is wrong. Let's remove it.
    if(SD.exists(old_log_name)) {
        SD.remove(old_log_name);
    }
    return true;
  }
  
  // Extract directory from new_log_name and ensure it exists
  String new_path = String(new_log_name);
  int last_slash = new_path.lastIndexOf('/');
  if (last_slash > 0) {
    String dir = new_path.substring(0, last_slash);
    if (!SD.exists(dir.c_str())) {
      M5_LOGD("Creating directory '%s' for rename", dir.c_str());
      if (!SD.mkdir(dir.c_str())) {
        M5_LOGE("Failed to create directory '%s'", dir.c_str());
        return false;
      }
    }
  }
  
  bool success = SD.rename(old_log_name, new_log_name);
  
  // Verify the renamed file exists
  if (success && !SD.exists(new_log_name)) {
    M5_LOGE("Rename reported success but file '%s' does not exist", new_log_name);
    success = false;
  }
  
  if (success) {
    M5_LOGD("Successfully renamed '%s' to '%s'", old_log_name, new_log_name);
  } else {
    M5_LOGE("Failed to rename '%s' to '%s'", old_log_name, new_log_name);
  }

  return success;
}

bool SDInterface::delete_log(const char* log_name) {
  if (!ready()) {
    return false;
  }

  return SD.remove(log_name);
}

void SDInterface::delete_directory(const char* dirPath) {
  File dir = SD.open(dirPath);
  if (!dir) {
    M5_LOGD("Directory does not exist or could not be opened.");
    return;
  }

  if (!dir.isDirectory()) {
    M5_LOGD("The specified path is not a directory.");
    dir.close();
    return;
  }

  // Iterate through all files in the directory
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      // No more files in the directory
      break;
    }

    // Construct the full path
    String fullPath = String(dirPath) + "/" + entry.name();

    if (entry.isDirectory()) {
      // Recursively delete subdirectories
      M5_LOGD("Recursively deleting directory: %s", fullPath.c_str());
      delete_directory(fullPath.c_str());

      // Attempt to remove the directory itself after its contents are deleted
      if (!SD.rmdir(fullPath.c_str())) {
        M5_LOGD("Failed to remove directory: %s", fullPath.c_str());
      } else {
        M5_LOGD("Successfully removed directory: %s", fullPath.c_str());
      }
    } else {
      // Try to delete the file
      if (!SD.remove(fullPath.c_str())) {
        M5_LOGD("Failed to delete file: %s", fullPath.c_str());
      } else {
        M5_LOGD("Successfully deleted file: %s", fullPath.c_str());
      }
    }
    entry.close();
  }

  dir.close();

  // Finally, remove the empty directory itself
  if (SD.rmdir(dirPath)) {
    M5_LOGD("Directory deleted successfully: %s", dirPath);
  } else {
    M5_LOGD("Failed to delete the directory: %s", dirPath);
  }
}

File SDInterface::get_file(const char* log_name) {
  return SD.open(log_name, FILE_READ);
}
