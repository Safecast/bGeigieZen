#include "battery_indicator.h"
#include "drivers/battery_mapping.h"
#include "drivers/axp2101_utils.h"

BatteryIndicator::BatteryIndicator() : Worker<BatteryStatus>(1000) {
}

bool BatteryIndicator::activate(bool retry) {
  // Configure AXP2101 coulomb counter once (4000 mAh preset). Ignore failure silently.
  static bool configured = false;
  if (!configured) {
    axp2101::configure_coulomb_counter(4000);
    configured = true;
  }
  // TODO: double check power button config, can now do through M5 unified lib
  return true;
}

int8_t BatteryIndicator::produce_data() {
  data.isCharging = M5.Power.isCharging();
  uint16_t voltage_mv = M5.Power.getBatteryVoltage(); // millivolts
  data.voltage = voltage_mv / 1000.0f;

  // Current (mA), positive when discharging, negative when charging
  data.current_mA = M5.Power.getBatteryCurrent();

  // Convert voltage to percentage using discharge curve mapping
  data.percentage = static_cast<int32_t>(BatteryMapping::voltage_to_percentage(voltage_mv) + 0.5f);
  return e_worker_data_read;
}
