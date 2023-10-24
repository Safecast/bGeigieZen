/** @brief Realtime clock IC handler for Core2 
 * 
 * Acts as a backup provider of realtime when no GNSS date-time available.
 * The BM8563 (a.k.a. PCF8563) RTC is only present on the M5Stack Core2.
 * Based on BM8563_Get.ino example in the library
*/

#include "rtc_connector.h"

RtcConnector::RtcConnector() : Worker<RtcData>() {
}

/**
 * @return true if initialized RTC library, false if no connection to the IC.
*/
bool RtcConnector::activate(bool retry) {

#ifdef M5_CORE2
DEBUG_PRINTLN("Activating RTC Connector, SDA, SCL");
DEBUG_PRINT(BM8563_I2C_SDA);
DEBUG_PRINTLN(BM8563_I2C_SCL);
  Wire1.begin(BM8563_I2C_SDA, BM8563_I2C_SCL);
  rtc.begin();

DEBUG_PRINTLN("Initialize RTC data to all zero");

  data.rtc_low_voltage = rtc.getVoltLow();
  data.year = 0;
  data.month = 0;
  data.day = 0;
  data.hour = 0;
  data.minute = 0;
  data.second = 0;

  return true;
#else
  return false;
#endif
}

int8_t RtcConnector::produce_data() {
#ifdef M5_CORE2
  rtc.getDate(&dateStruct);
  rtc.getTime(&timeStruct);
  data.rtc_low_voltage = rtc.getVoltLow();

  data.second = timeStruct.seconds;
  data.minute = timeStruct.minutes;
  data.hour = timeStruct.hours;
  data.day = dateStruct.date;
  data.month = dateStruct.month;
  data.year = dateStruct.year;

  return e_worker_data_read;
#else
  return e_worker_idle;
#endif
}
