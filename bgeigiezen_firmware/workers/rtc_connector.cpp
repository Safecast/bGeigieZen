/** @brief Realtime clock IC handler for Core2 
 * 
 * Acts as a backup provider of realtime when no GNSS date-time available.
 * The PCF8563 IC is only present on the M5Stack Core2.
 * Based on PCF8563_Alarms example in RTC by Michael Miller

*/

#include "rtc_connector.h"

RtcConnector::RtcConnector() : Worker<RtcData>() {
}

/**
 * @return true if initialized RTC library, false if no connection to the IC.
*/
bool RtcConnector::activate(bool retry) {
  // Until we can read the VL bit in the RTC,
  // mark the voltage low bit invalid to start
  data.rtc_power_good = false;
  data.year = 0;
  data.month = 0;
  data.day = 0;
  data.hour = 0;
  data.minute = 0;
  data.second = 0;

  return true;
}

int8_t RtcConnector::produce_data() {
  M5.Rtc.GetTime(&zen_rtc_time);
  M5.Rtc.GetDate(&zen_rtc_date);
  data.second = zen_rtc_time.Seconds;
  data.minute = zen_rtc_time.Minutes;
  data.hour = zen_rtc_time.Hours;
  data.day = zen_rtc_date.Date;
  data.month = zen_rtc_date.Month;
  data.year = zen_rtc_date.Year;

  // But until we can read the VL bit in the RTC, this has to stay:
  data.rtc_power_good = false;

  return e_worker_data_read;
}
