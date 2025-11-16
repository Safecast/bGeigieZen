#ifndef BGEIGIEZEN_BATTERYINDICATOR_H_
#define BGEIGIEZEN_BATTERYINDICATOR_H_

#include <M5Unified.hpp>
#include <Worker.hpp>

/**
 * Relevant battery status
 */
struct BatteryStatus {
<<<<<<< HEAD
  int32_t percentage; // Now mapped from voltage
  float voltage;      // Battery voltage in volts
  float current_mA;   // Battery current in milliamps (+ discharge, - charge)
=======
  int32_t percentage;
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
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
