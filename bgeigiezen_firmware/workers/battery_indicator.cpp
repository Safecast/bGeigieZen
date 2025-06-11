#include "battery_indicator.h"

BatteryIndicator::BatteryIndicator() : Worker<BatteryStatus>(1000) {
}

bool BatteryIndicator::activate(bool retry) {
  // TODO: double check power button config, can now do through M5 unified lib
  return true;
}

int8_t BatteryIndicator::produce_data() {
  data.isCharging = M5.Power.isCharging();
  data.voltage = M5.Power.getBatteryVoltage() / 1000.0f; // Convert mV to V if needed
  // Map voltage to percentage: 4.2V = 100%, 3.0V = 0%
  float pct = (data.voltage - 3.0f) / (4.2f - 3.0f) * 100.0f;
  if (pct > 100.0f) pct = 100.0f;
  if (pct < 0.0f) pct = 0.0f;
  data.percentage = static_cast<int32_t>(pct + 0.5f); // Round to nearest int
  return e_worker_data_read;
}
