/** @brief Realtime clock IC handler for Core2 only.
 * 
 * Acts as a backup provider of realtime when no GNSS date-time available.
 * The PCF8563 IC is only present on the M5Stack Core2.
 * 
*/

#ifndef BGEIGIEZEN_RTC_CONNECTOR_H_
#define BGEIGIEZEN_RTC_CONNECTOR_H_

/** @note Temporarily using tanakamasayuki/I2C BM8563 RTC library
 * M5Core2 includes a driver for the 8563 RTC chip, but it does not give
 * you access to the Voltage Low bit (bit 7 of the seconds register) to
 * detect a low voltage event that might invalidate the date-time values.
 * See pull request #138 in m5stack/M5Core2
 * https://github.com/m5stack/M5Core2/pull/138
 * Core2 RTC GPIO pins SDA=21  SCL=22 (bit bashing?)
*/

#include "gps_connector.h"
#include <Worker.hpp>
#include <user_config.h>

struct RtcData {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  bool valid;
};

/**
 * RTC device worker, produces the current date and time.
 */
class DateTimeProvider : public ProcessWorker<RtcData> {
 public:

  explicit DateTimeProvider();

  virtual ~DateTimeProvider() = default;

  bool activate(bool retry) override;

  int8_t produce_data(const worker_map_t& workers) override;

 private:

  // read dt: System -> Data
  void system_to_data();

  // save dt: GPS / RTC -> System (-> RTC)
  void gps_to_system(const GnssData& gps_data);
  void rtc_to_system();
  void save_to_system(tm& sys_time);
  void system_to_rtc();
  void invalidate_sys_time();

  tm _sys_time;
  uint32_t _last_sys_set;
};

#endif //BGEIGIEZEN_RTC_CONNECTOR_H_
