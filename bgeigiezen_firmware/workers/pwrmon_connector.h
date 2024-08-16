/** @brief INA3221 voltage and current monitor IC handler for Core2 
 * 
 * Separate instrumentation work to determine power requirements
 * for remote or solar powered operation.
 */

#ifndef BGEIGIEZEN_PWRMON_CONNECTOR_H_
#define BGEIGIEZEN_PWRMON_CONNECTOR_H_

/** @note Temporarily using tanakamasayuki/I2C BM8563 RTC library
 * M5Core2 includes a driver for the 8563 RTC chip, but it does not give
 * you access to the Voltage Low bit (bit 7 of the seconds register) to
 * detect a low voltage event that might invalidate the date-time values.
 * See pull request #138 in m5stack/M5Core2
 * https://github.com/m5stack/M5Core2/pull/138
 * Core2 RTC GPIO pins SDA=21  SCL=22 (bit bashing?)
*/

// #ifdef M5_CORE2
// #include <M5Core2.h>
// #endif
#include "debugger.h"
#include <Worker.hpp>
#include <user_config.h>

struct PwrmonData {
  // Voltage & current
  float ibatt;  // Li-Ion battery when not on USB or charging
  float vbatt;
  float iboost;  // AXP2101 boost converter for 5V devices
  float vboost;
  float ibus;  // Supply from USB
  float vbus;
};

/**
 * Power monitor device worker, produces the voltage and current data.
 */
class PwrmonConnector : public ProcessWorker<PwrmonData> {
 public:

  explicit PwrmonConnector();

  virtual ~PwrmonConnector() = default;

  bool activate(bool retry) override;

  int8_t produce_data(const worker_map_t& workers) override;

 private:

};

#endif //BGEIGIEZEN_PWRMON_CONNECTOR_H_
