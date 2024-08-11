#include "battery_indicator.h"

#include <M5Unified.hpp>

BatteryIndicator::BatteryIndicator() : Worker<BatteryStatus>(1000) {
}

bool BatteryIndicator::activate(bool retry) {
  // TODO: double check power button config, can now do through M5 unified lib
  return true;
}

int8_t BatteryIndicator::produce_data() {

  data.isCharging = M5.Power.isCharging();
  data.percentage = M5.Power.getBatteryLevel();
  return e_worker_data_read;
}
