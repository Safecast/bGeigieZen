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

  // Current (mA), positive when discharging, negative when charging. If
  // M5Unified returns 0 (not implemented on some boards), fallback to AXP2101
  // direct ADC reading.
  int32_t current = M5.Power.getBatteryCurrent();
  if (current == 0) {
    current = axp2101::get_battery_current_mA();
  }
  data.current_mA = static_cast<float>(current);

  // Convert voltage to percentage using discharge curve mapping
  data.percentage = static_cast<int32_t>(BatteryMapping::voltage_to_percentage(voltage_mv) + 0.5f);
  return e_worker_data_read;
}
