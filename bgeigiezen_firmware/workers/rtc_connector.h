/** @brief Realtime clock IC handler for Core2 only.
 * 
 * Acts as a backup provider of realtime when no GNSS date-time available.
 * The PCF8563 IC is only present on the M5Stack Core2.
 * 
*/

#ifndef BGEIGIEZEN_RTC_CONNECTOR_H_
#define BGEIGIEZEN_RTC_CONNECTOR_H_

#include <M5Core2.h>  // Instance Rtc already defined and initialized
#include <RTC.h>  // M5 Core2 RTC
#include <Worker.hpp>
#include <user_config.h>

struct RtcData {
  // Date & Time
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

  // Confidence 
  bool rtc_power_good;  // VL bit is true
};

/**
 * RTC device worker, produces the current date and time.
 */
class RtcConnector : public Worker<RtcData> {
 public:

  explicit RtcConnector();

  virtual ~RtcConnector() = default;

  bool activate(bool retry) override;

  int8_t produce_data() override;

 private:
  RTC_TimeTypeDef zen_rtc_time;
  RTC_DateTypeDef zen_rtc_date;

};

#endif //BGEIGIEZEN_RTC_CONNECTOR_H_
