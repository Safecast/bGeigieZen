#ifndef BGEIGIEZEN_BATTERYINDICATOR_HPP
#define BGEIGIEZEN_BATTERYINDICATOR_HPP

#include <Worker.hpp>
#include <stdint.h>

/**
 * Relevant battery status
 */
struct BatteryStatus {
  uint8_t percentage;
  bool isCharging;
};

/**
 * Battery indicator worker, uses the M5 library to check up on the battery and charging status
 */
class BatteryIndicator : public Worker<BatteryStatus> {
 public:
  explicit BatteryIndicator();
  virtual ~BatteryIndicator() = default;

  bool activate(bool retry) override;

  int8_t produce_data() override;

 private:

};

#endif //BGEIGIEZEN_BATTERYINDICATOR_HPP
