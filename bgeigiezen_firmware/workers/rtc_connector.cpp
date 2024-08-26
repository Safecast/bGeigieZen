/** @brief Realtime clock IC handler for Core2 
 * 
 * Acts as a backup provider of realtime when no GNSS date-time available.
 * The BM8563 (a.k.a. PCF8563) RTC is only present on the M5Stack Core2.
 * Based on BM8563_Get.ino example in the library
*/

#include "rtc_connector.h"
#include "gps_connector.h"
#include "identifiers.h"


DateTimeProvider::DateTimeProvider() : ProcessWorker<RtcData>({.valid=false}, 1000), _sys_time{}, _last_sys_set(0) {
}

/**
 * @return true if initialized RTC library, false if no connection to the IC.
*/
bool DateTimeProvider::activate(bool retry) {

  if (M5.getBoard() != m5::board_t::board_M5Stack) {
    M5.Rtc.begin();
  }

  if (M5.Rtc.isEnabled()) {
    rtc_to_system();
  } else {
    M5_LOGD("RTC unit not found");
    // Start with invalid data
    invalidate_sys_time();
  }

  return true;
}

int8_t DateTimeProvider::produce_data(const worker_map_t& workers) {
  const auto& gps_data = workers.worker<GpsConnector>(k_worker_gps_connector)->get_data();
  if (gps_data.date_valid && gps_data.time_valid && (_last_sys_set == 0 || _last_sys_set + 1000*60*60*6 < millis())) {
    gps_to_system(gps_data);
  }

  system_to_data();
  return e_worker_data_read;
}

void DateTimeProvider::system_to_data() {
  time_t now = time(nullptr);
  localtime_r(&now, &_sys_time);

  data.year = _sys_time.tm_year + 1900;
  data.month = _sys_time.tm_mon + 1;
  data.day = _sys_time.tm_mday;
  data.hour = _sys_time.tm_hour;
  data.minute = _sys_time.tm_min;
  data.second = _sys_time.tm_sec;
  data.valid = data.year > 2023 && data.year < 2036;
}

void DateTimeProvider::gps_to_system(const GnssData& gps_data) {
  if (gps_data.time_valid && gps_data.date_valid) {
    _last_sys_set = millis();
    tm sys_time{};
    sys_time.tm_isdst = -1;
    sys_time.tm_year = gps_data.year - 1900;
    sys_time.tm_mon  = gps_data.month - 1;
    sys_time.tm_mday = gps_data.day;
    sys_time.tm_hour = gps_data.hour;
    sys_time.tm_min  = gps_data.minute;
    sys_time.tm_sec  = gps_data.second;
    save_to_system(sys_time);

    M5_LOGD("Set system time from GPS: %04d-%02d-%02d %02d:%02d:%02d",
            gps_data.year, gps_data.month, gps_data.day,
            gps_data.hour, gps_data.minute, gps_data.second);
  }
}

void DateTimeProvider::rtc_to_system() {
  if (M5.Rtc.isEnabled()) {
    m5::rtc_datetime_t rtc_datetime;
    M5.Rtc.getDateTime(&rtc_datetime);
    if (rtc_datetime.date.year < 2024 || rtc_datetime.date.year > 2035) {
      // RTC returned invalid data
      M5_LOGD("RTC invalid year, reset sys time");
      return invalidate_sys_time();
    }
    tm sys_time{};
    sys_time.tm_isdst = -1;
    sys_time.tm_year = static_cast<int>(rtc_datetime.date.year) - 1900;
    sys_time.tm_mon  = static_cast<uint8_t>(rtc_datetime.date.month) - 1;
    sys_time.tm_mday = static_cast<uint8_t>(rtc_datetime.date.date);
    sys_time.tm_hour = static_cast<uint8_t>(rtc_datetime.time.hours);
    sys_time.tm_min  = static_cast<uint8_t>(rtc_datetime.time.minutes);
    sys_time.tm_sec  = static_cast<uint8_t>(rtc_datetime.time.seconds);

    M5_LOGD("Set system time from RTC: %04d-%02d-%02d %02d:%02d:%02d",
            rtc_datetime.date.year, rtc_datetime.date.month, rtc_datetime.date.date,
            rtc_datetime.time.hours, rtc_datetime.time.minutes, rtc_datetime.time.seconds);

    save_to_system(sys_time);
  }
}

void DateTimeProvider::system_to_rtc() {
  if (M5.Rtc.isEnabled()) {
    time_t now = time(nullptr);
    localtime_r(&now, &_sys_time);

    M5.Rtc.setDateTime(&_sys_time);

    m5::rtc_datetime_t rtc_datetime;
    M5.Rtc.getDateTime(&rtc_datetime);
    M5_LOGD("Set RTC from system: %04d-%02d-%02d %02d:%02d:%02d",
            rtc_datetime.date.year, rtc_datetime.date.month, rtc_datetime.date.date,
            rtc_datetime.time.hours, rtc_datetime.time.minutes, rtc_datetime.time.seconds);
  }
}

void DateTimeProvider::save_to_system(tm& sys_time) {
  // mktime(3) uses localtime, force UTC
  const char* device_tz = getenv("TZ");
  setenv("TZ", "GMT0", 1);
  tzset();

  timeval now{
      .tv_sec=mktime(&sys_time),
      .tv_usec=0
  };
  settimeofday(&now, nullptr);

  if (device_tz) {
    setenv("TZ", device_tz, 1);
    tzset();
  } else {
    unsetenv("TZ");
  }

  system_to_rtc();
}

void DateTimeProvider::invalidate_sys_time() {
  M5_LOGD("Set system time invalid: 2020-01-01 00:00:00");

  tm sys_time{};
  sys_time.tm_isdst = -1;
  sys_time.tm_year = 2020 - 1900;
  sys_time.tm_mon  = 0;
  sys_time.tm_mday = 1;
  sys_time.tm_hour = 0;
  sys_time.tm_min  = 0;
  sys_time.tm_sec  = 0;
  save_to_system(sys_time);
}
