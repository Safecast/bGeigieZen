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
#include <M5Core2.h>
#include <I2C_BM8563.h> // Bypass instance Rtc from M5Core2
#include <Worker.hpp>
#include <user_config.h>
#include "debugger.h"

struct RtcData {
  // Date & Time
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

  // RTC VL_SECONDS register (0x02_ bit 7 is VL bit:
  //   0 = clock integrity guaranteed
  //   1 = low power event detected
  bool rtc_low_voltage; // 1=clock integrity not guaranteed
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
  I2C_BM8563 rtc{I2C_BM8563_DEFAULT_ADDRESS, Wire1};

  I2C_BM8563_DateTypeDef dateStruct;
  I2C_BM8563_TimeTypeDef timeStruct;

};

#endif //BGEIGIEZEN_RTC_CONNECTOR_H_
