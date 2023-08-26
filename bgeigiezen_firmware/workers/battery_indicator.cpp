#include "battery_indicator.h"

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

BatteryIndicator::BatteryIndicator() : Worker<BatteryStatus>(1000) {
}

bool BatteryIndicator::activate(bool retry) {
  return true;
}

int8_t BatteryIndicator::produce_data() {

#ifdef M5_CORE2
  data.isCharging = M5.Axp.isCharging();
  float batVoltage = M5.Axp.GetBatVoltage();
  float batPercentage = ( batVoltage < 3.2f ) ? 0 : ( batVoltage - 3.2f ) * 100;
  if (batPercentage > 100) {
    data.percentage = 100;
  } else {
    data.percentage = static_cast<uint8_t>(batPercentage);
  }
//  data.voltage = M5.Axp.GetBatVoltage();
#elif M5_BASIC
  data.isCharging = M5.Power.isCharging();
  data.percentage = M5.Power.getBatteryLevel();
#endif
  return e_worker_data_read;
}
