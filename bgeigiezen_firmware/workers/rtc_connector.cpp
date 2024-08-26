/** @brief Realtime clock IC handler for Core2 
 * 
 * Acts as a backup provider of realtime when no GNSS date-time available.
 * The BM8563 (a.k.a. PCF8563) RTC is only present on the M5Stack Core2.
 * Based on BM8563_Get.ino example in the library
*/

#include "rtc_connector.h"
#include "gps_connector.h"
#include "identifiers.h"


DateTimeProvider::DateTimeProvider() : ProcessWorker<RtcData>({.year=0, .month=0, .day=0, .hour=0, .minute=0, .second=0, .valid=false}, 1000), _t_st(), _last_sys_set(0) {
}

/**
 * @return true if initialized RTC library, false if no connection to the IC.
*/
bool DateTimeProvider::activate(bool retry) {

  if (M5.getBoard() != m5::board_t::board_M5Stack) {
    M5.Rtc.begin();
  }

  if (M5.Rtc.isEnabled()) {
    rtc_to_data();
  } else {
    M5_LOGD("RTC unit not found");
    // Start with invalid data
    data_to_system(false);
  }

  return true;
}

int8_t DateTimeProvider::produce_data(const worker_map_t& workers) {
  const auto& gps_data = workers.worker<GpsConnector>(k_worker_gps_connector)->get_data();
  if (gps_data.date_valid && gps_data.time_valid && (_last_sys_set == 0 || _last_sys_set + 1000*60*60*6 < millis())) {
    gps_to_data(gps_data);
  } else {
    system_to_data();
  }

  return e_worker_data_read;
}

void DateTimeProvider::system_to_data() {
  time_t now = time(nullptr);
  localtime_r(&now, &_t_st);

  data.year = _t_st.tm_year + 1900;
  data.month = _t_st.tm_mon + 1;
  data.day = _t_st.tm_mday;
  data.hour = _t_st.tm_hour;
  data.minute = _t_st.tm_min;
  data.second = _t_st.tm_sec;
}

void DateTimeProvider::gps_to_data(const GnssData& gps_data) {
  if (gps_data.time_valid && gps_data.date_valid) {
    data.valid = true;
    data.year = gps_data.year;
    data.month = gps_data.month;
    data.day = gps_data.day;
    data.hour = gps_data.hour;
    data.minute = gps_data.minute;
    data.second = gps_data.second;
    data_to_system(false);
  }
}

void DateTimeProvider::rtc_to_data() {
  if (M5.Rtc.isEnabled()) {
    m5::rtc_datetime_t rtc_datetime;
    M5.Rtc.getDateTime(&rtc_datetime);
    data.year = static_cast<int>(rtc_datetime.date.year);
    data.month = static_cast<uint8_t>(rtc_datetime.date.month);
    data.day = static_cast<uint8_t>(rtc_datetime.date.date);
    data.hour = static_cast<uint8_t>(rtc_datetime.time.hours);
    data.minute = static_cast<uint8_t>(rtc_datetime.time.minutes);
    data.second = static_cast<uint8_t>(rtc_datetime.time.seconds);
    data.valid = data.year > 2023;
    data_to_system(true);
  }
}

void DateTimeProvider::system_to_rtc() {
  if (M5.Rtc.isEnabled() && data.valid) {
    time_t now = time(nullptr);
    localtime_r(&now, &_t_st);

    M5.Rtc.setDateTime(&_t_st);

    m5::rtc_datetime_t rtc_datetime;
    M5.Rtc.getDateTime(&rtc_datetime);
  }
}

void DateTimeProvider::data_to_system(bool save_rtc) {
  // mktime(3) uses localtime, force UTC
  if (!data.valid || data.year < 2024 || data.year > 2035) {
    M5_LOGD("Data not valid, reset to default date");
    data.valid = false;
    data.year = 2020;
    data.month = 1;
    data.day = 1;
    data.hour = 0;
    data.minute = 0;
    data.second = 0;
  } else {
    data.valid = true;
  }
  char * device_tz = getenv("TZ");
  setenv("TZ", "GMT0", 1);
  tzset();

  tm t_st;
  t_st.tm_isdst = -1;
  t_st.tm_year = data.year - 1900;
  t_st.tm_mon  = data.month - 1;
  t_st.tm_mday = data.day;
  t_st.tm_hour = data.hour;
  t_st.tm_min  = data.minute;
  t_st.tm_sec  = data.second;

  timeval now{
      .tv_sec=mktime(&_t_st),
      .tv_usec=0
  };
  settimeofday(&now, nullptr);

  if (device_tz)
  {
    setenv("TZ", device_tz, 1);
    tzset();
  } else {
    unsetenv("TZ");
  }
  if (save_rtc) {
    system_to_rtc();
  }
}
