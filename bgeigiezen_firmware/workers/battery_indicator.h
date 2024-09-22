#ifndef BGEIGIEZEN_BATTERYINDICATOR_H_
#define BGEIGIEZEN_BATTERYINDICATOR_H_

#include <M5Unified.hpp>
#include <Worker.hpp>

/**
 * Relevant battery status
 */
struct BatteryStatus {
  int32_t percentage;
  m5::Power_Class::is_charging_t isCharging;
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

#endif //BGEIGIEZEN_BATTERYINDICATOR_H_
