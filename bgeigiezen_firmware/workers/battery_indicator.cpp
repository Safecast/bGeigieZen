#include "battery_indicator.h"
<<<<<<< HEAD
#include "drivers/battery_mapping.h"
// axp2101_utils disabled
//#include "drivers/axp2101_utils.h"

#if 0
namespace {
void dump_pmic_current_regs() {
  uint8_t en1 = 0, h = 0, l = 0;
  axp2101::read_register(0x84, en1);           // ADC_EN1
  axp2101::read_register(0x7A, h);             // BAT_DISCHG_CUR_H
  axp2101::read_register(0x7B, l);             // BAT_DISCHG_CUR_L
  M5_LOGI("PMIC diagnostic: ADC_EN1=0x%02X DISCH_CUR=0x%02X%02X getBatteryCurrent()=%d mA",
          en1, h, l, M5.Power.getBatteryCurrent());
}
}
#endif // disable axp2101 util diagnostic
=======
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416

BatteryIndicator::BatteryIndicator() : Worker<BatteryStatus>(1000) {
}

bool BatteryIndicator::activate(bool retry) {
<<<<<<< HEAD
  // Configure AXP2101 coulomb counter once (4000 mAh preset). Ignore failure silently.
  static bool configured = false;
  if (!configured) {

    configured = true;
  }
=======
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
  // TODO: double check power button config, can now do through M5 unified lib
  return true;
}

int8_t BatteryIndicator::produce_data() {
<<<<<<< HEAD
  data.isCharging = M5.Power.isCharging();
  uint16_t voltage_mv = M5.Power.getBatteryVoltage(); // millivolts
  data.voltage = voltage_mv / 1000.0f;

  // Current (mA), positive when discharging, negative when charging. If
  // M5Unified returns 0 (not implemented on some boards), fallback to AXP2101
  // direct ADC reading.
  int32_t current = M5.Power.getBatteryCurrent();

  data.current_mA = static_cast<float>(current);


  // Convert voltage to percentage using discharge curve mapping
  data.percentage = static_cast<int32_t>(BatteryMapping::voltage_to_percentage(voltage_mv) + 0.5f);
=======

  data.isCharging = M5.Power.isCharging();
  data.percentage = M5.Power.getBatteryLevel();
>>>>>>> 4d1f50fa8cf254334dd79afac188923947dfc416
  return e_worker_data_read;
}
