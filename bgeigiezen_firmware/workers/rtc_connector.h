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

#ifdef M5_CORE2
#include <M5Core2.h>
#include <I2C_BM8563.h>
#endif
#include "debugger.h"
#include "gps_connector.h"
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

  // RTC VL_SECONDS register (0x02_ bit 7 is VL bit:
  //   false = clock integrity guaranteed
  //   true = low power event detected (clock integrity not guaranteed)
  bool low_voltage;
  // true if year >= 2023 and low_voltage false (in case init with 2k value)
  bool valid;
};

/**
 * RTC device worker, produces the current date and time.
 */
class RtcConnector : public ProcessWorker<RtcData> {
 public:

  explicit RtcConnector();

  virtual ~RtcConnector() = default;

  bool activate(bool retry) override;

  int8_t produce_data(const worker_map_t& workers) override;

 private:

  void set_from_gps(const GnssData& gps_data);

#ifdef M5_CORE2
  I2C_BM8563 rtc;

  I2C_BM8563_DateTypeDef dateStruct;
  I2C_BM8563_TimeTypeDef timeStruct;
#endif

};

#endif //BGEIGIEZEN_RTC_CONNECTOR_H_
