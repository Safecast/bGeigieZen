/** @brief Realtime clock IC handler for Core2 
 * 
 * Acts as a backup provider of realtime when no GNSS date-time available.
 * The BM8563 (a.k.a. PCF8563) RTC is only present on the M5Stack Core2.
 * Based on BM8563_Get.ino example in the library
*/

#include "rtc_connector.h"
#include "gps_connector.h"
#include "identifiers.h"

#ifdef M5_CORE2
RtcConnector::RtcConnector() : ProcessWorker<RtcData>({.year=0, .month=0, .day=0, .hour=0, .minute=0, .second=0, .low_voltage=false, .valid=false}, 1000),
                               rtc(I2C_BM8563_DEFAULT_ADDRESS, Wire1),
                               dateStruct({.weekDay=0, .month=0, .date=0, .year=0}),
                               timeStruct({.hours=0, .minutes=0, .seconds=0}) {
}
#else
RtcConnector::RtcConnector() : ProcessWorker<RtcData>({.year=0, .month=0, .day=0, .hour=0, .minute=0, .second=0, .low_voltage=false, .valid=false}, 1000) {
}
#endif

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
  data.low_voltage = rtc.getVoltLow();
  rtc.getDate(&dateStruct);
  rtc.getTime(&timeStruct);
  return true;
#else
  return false;
#endif
}

int8_t RtcConnector::produce_data(const worker_map_t& workers) {
#ifdef M5_CORE2
  rtc.getDate(&dateStruct);
  rtc.getTime(&timeStruct);
  data.low_voltage = rtc.getVoltLow();

  if (!data.low_voltage && dateStruct.year >= 2023 && dateStruct.year < 2050) {  // just to make sure year isn't absurdly high
    // Data is valid
    data.valid = true;
    data.second = timeStruct.seconds;
    data.minute = timeStruct.minutes;
    data.hour = timeStruct.hours;
    data.day = dateStruct.date;
    data.month = dateStruct.month;
    data.year = dateStruct.year;
    return e_worker_data_read;
  } else {
    // Unable to validate data, check gps
    const auto& gps_data = workers.worker<GpsConnector>(k_worker_gps_connector)->get_data();
    if (gps_data.time_valid && gps_data.date_valid) {
      set_from_gps(gps_data);
      return e_worker_data_read;
    } else {
      // No valid options
      data.valid = false;
      data.second = 0;
      data.minute = 0;
      data.hour = 0;
      data.day = 0;
      data.month = 0;
      data.year = 0;
    }
  }

#else
  // TODO: handle M5core
  // Try gps
  const auto& gps_data = workers.worker<GpsConnector>(k_worker_gps_connector)->get_data();
  if (gps_data.time_valid && gps_data.date_valid) {
    set_from_gps(gps_data);
    return e_worker_data_read;
  } else {
    data.valid = false;
    data.second = 0;
    data.minute = 0;
    data.hour = 0;
    data.day = 0;
    data.month = 0;
    data.year = 0;
  }
#endif

  return e_worker_idle;
}

void RtcConnector::set_from_gps(const GnssData& gps_data) {
  data.valid = true;
  data.second = gps_data.second;
  data.minute = gps_data.minute;
  data.hour = gps_data.hour;
  data.day = gps_data.day;
  data.month = gps_data.month;
  data.year = gps_data.year;

#ifdef M5_CORE2
  // Set RTC values by gps
  rtc.setDate(&dateStruct);
  rtc.setTime(&timeStruct);
#endif

}
