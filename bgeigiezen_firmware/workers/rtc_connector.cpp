/** @brief Realtime clock IC handler for Core2 
 * 
 * Acts as a backup provider of realtime when no GNSS date-time available.
 * The BM8563 (a.k.a. PCF8563) RTC is only present on the M5Stack Core2.
 * Based on BM8563_Get.ino example in the library
*/

#include "rtc_connector.h"
#include "gps_connector.h"
#include "identifiers.h"

// 1 jan 2023
#define UNIX_DEFAULT 1672527600

#ifdef M5_CORE2
RtcConnector::RtcConnector() : ProcessWorker<RtcData>({.year=0, .month=0, .day=0, .hour=0, .minute=0, .second=0, .low_voltage=false, .valid=false}, 1000),
                               rtc(I2C_BM8563_DEFAULT_ADDRESS, Wire1),
                               dateStruct({.weekDay=0, .month=0, .date=0, .year=0}),
                               timeStruct({.hours=0, .minutes=0, .seconds=0}),
                               tv{.tv_sec=UNIX_DEFAULT} {
}
#else
RtcConnector::RtcConnector() : ProcessWorker<RtcData>({.year=0, .month=0, .day=0, .hour=0, .minute=0, .second=0, .low_voltage=false, .valid=false}, 1000), tv{.tv_sec=UNIX_DEFAULT} {
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
#else
#endif
  // set current day/time
  settimeofday(&tv, nullptr);
  return true;
}

int8_t RtcConnector::produce_data(const worker_map_t& workers) {
  auto ret_status = e_worker_idle;
#ifdef M5_CORE2
  rtc.getDate(&dateStruct);
  rtc.getTime(&timeStruct);
  data.low_voltage = rtc.getVoltLow();

  const auto& gps_data = workers.worker<GpsConnector>(k_worker_gps_connector)->get_data();

  if (!data.low_voltage && dateStruct.year >= 2023 && dateStruct.year < 2050) {  // just to make sure year isn't absurdly high
    // Data is valid
    data.valid = true;
    data.second = timeStruct.seconds;
    data.minute = timeStruct.minutes;
    data.hour = timeStruct.hours;
    data.day = dateStruct.date;
    data.month = dateStruct.month;
    data.year = dateStruct.year;

    ret_status = e_worker_data_read;
  } else {
    // Unable to validate data, check gps
    if (gps_data.time_valid && gps_data.date_valid) {
      set_from_gps(gps_data);
      ret_status = e_worker_data_read;
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
  if (gps_data.date_valid) {
    set_from_gps(gps_data);
    ret_status = e_worker_data_read;
  } else if (data.valid) {
    // TODO: tick up if previous data valid, can we use the following as simple implementation?
    data.second = (data.second + 1) % 60;
    if (data.second == 0) {
      data.minute = (data.minute + 1) % 60;
      if (data.minute == 0) {
        data.hour = (data.hour + 1) % 24;
        if (data.hour == 0) {
          // hour ticked up to a new day, we skip that for now and let next gps data handle it
          data.valid = false;
        } else {
          ret_status = e_worker_data_read;
        }
      }
    }
  } else {
    // No valid date/time data available
    data.valid = false;
    data.second = 0;
    data.minute = 0;
    data.hour = 0;
    data.day = 0;
    data.month = 0;
    data.year = 0;
  }
#endif

  set_unix_data();
  return ret_status;
}

void RtcConnector::set_unix_data() {
  if (data.valid && data.year > 2020 && tv.tv_sec == UNIX_DEFAULT) {
    // Taken from ublox unix calculation
    uint32_t t = SFE_UBLOX_DAYS_FROM_1970_TO_2020;                                                                           // Jan 1st 2020 as days from Jan 1st 1970
    t += (uint32_t)SFE_UBLOX_DAYS_SINCE_2020[data.year - 2020];                                             // Add on the number of days since 2020
    t += (uint32_t)SFE_UBLOX_DAYS_SINCE_MONTH[data.year % 4 == 0 ? 0 : 1][data.month - 1]; // Add on the number of days since Jan 1st
    t += (uint32_t)data.day - 1;                                                                            // Add on the number of days since the 1st of the month
    t *= 24;                                                                                                                 // Convert to hours
    t += (uint32_t)data.hour;                                                                               // Add on the hour
    t *= 60;                                                                                                                 // Convert to minutes
    t += (uint32_t)data.minute;                                                                                // Add on the minute
    t *= 60;                                                                                                                 // Convert to seconds
    t += (uint32_t)data.second;                                                                                // Add on the second
    tv.tv_sec = t;
    settimeofday(&tv, nullptr);
//    setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/ 3", 1); // https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
  }
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


