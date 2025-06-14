#include "battery_indicator.h"
#include "drivers/battery_mapping.h"

BatteryIndicator::BatteryIndicator() : Worker<BatteryStatus>(1000) {
}

bool BatteryIndicator::activate(bool retry) {
  // TODO: double check power button config, can now do through M5 unified lib
  return true;
}

int8_t BatteryIndicator::produce_data() {
  data.isCharging = M5.Power.isCharging();
  data.voltage = M5.Power.getBatteryVoltage() / 1000.0f; // Convert mV to V
  // Convert voltage to percentage using the mapping table
  data.percentage = static_cast<int32_t>(BatteryMapping::voltage_to_percentage(data.voltage * 1000.0f) + 0.5f); // Round to nearest int
  return e_worker_data_read;
}
