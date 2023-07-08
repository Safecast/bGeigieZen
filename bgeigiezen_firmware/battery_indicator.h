#ifndef BGEIGIEZEN_BATTERYINDICATOR_HPP
#define BGEIGIEZEN_BATTERYINDICATOR_HPP

#include <Worker.hpp>
#include <stdint.h>

struct BatteryStatus {
  uint8_t percentage;
  bool isCharging;
};

/**
 * Geiger counter worker, produces CPM.
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
