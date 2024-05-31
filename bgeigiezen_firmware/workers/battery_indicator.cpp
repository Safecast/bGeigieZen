#include "battery_indicator.h"

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

BatteryIndicator::BatteryIndicator() : Worker<BatteryStatus>(1000) {
}

bool BatteryIndicator::activate(bool retry) {
  // Power off after holding side button 4 seconds instead of default 6s
  // Use AXP2101 6.13.2.24 REG 27：IRQLEVEL/OFFLEVEL/ONLEVEL setting (p.38)
  // Identify power management IC, act only if it's AXP2101
  // From AXP::begin() in AXP.cpp (0x03 is undocumented in AXP 2101 datasheet)
#ifdef M5_CORE2
  uint8_t val = Read8bit(0x03);
  uint8_t reg27 = 0;
  if (val == 0x4A) {
    reg27 = Read8bit(0x27);
    reg27 = reg27 & ~0b00001100; // bits[3:2] = 00 OFFLEVEL configuration = 4s 
    Write1Byte(0x27, reg27);

    //Set power charging to 1 Amp (moved from main.cpp)
    Write1Byte(AXP2101_ICC_CHARGER_SETTING_REG, 16);
  }
#endif
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
