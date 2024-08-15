#include "battery_indicator.h"
#include "debugger.h"

#ifdef M5_CORE2
#include <M5Core2.h>
#elif M5_BASIC
#include <M5Stack.h>
#endif

BatteryIndicator::BatteryIndicator() : Worker<BatteryStatus>(1000) {
}

// Initialize reading from INA3221 ammeter IC
// INA3221 I2C address 0x40
void BatteryIndicator::ina3221_init(void) {
  uint16_t ina_conf = M5.Axp.ina3221.getReg(INA3221_REG_CONF);
  uint16_t ina_mid = M5.Axp.ina3221.getManufID();
  DEBUG_PRINTF("INA3221: Config = %04x\n", ina_conf);
  DEBUG_PRINTF("INA3221: Manufacturer ID = %04x\n", ina_mid);

  // From Core2 v1.1 schematic p.4, shunt resistors 10 milliOhm
  M5.Axp.ina3221.setShuntRes(10L, 10L, 10L);
  // From Core2 v1.1 schematic p.2, filter resistors 10 Ohm
  M5.Axp.ina3221.setFilterRes(10L, 10L, 10L);

  // Sets operating mode to continious [sic]
  M5.Axp.ina3221.setModeContinious();
  // Enables shunt-voltage measurement
  M5.Axp.ina3221.setShuntMeasEnable();
  // Enables bus-voltage measurement
  M5.Axp.ina3221.setBusMeasEnable();

  // Gets battery current in A, compensated with calculated offset voltage.
  float ina_ibatt = M5.Axp.ina3221.getCurrentCompensated(INA3221_CH1);
  DEBUG_PRINTF("INA3221: Battery Current (Compensated) = %f\n", ina_ibatt);
  // Gets battery voltage in V.
  float ina_vbatt = M5.Axp.ina3221.getVoltage(INA3221_CH1);
  DEBUG_PRINTF("INA3221: Battery Voltage = %f\n", ina_vbatt);

  // Gets 5V boost current in A, compensated with calculated offset voltage.
  float ina_iboost = M5.Axp.ina3221.getCurrentCompensated(INA3221_CH2);
  DEBUG_PRINTF("INA3221: 5V Boost Current (Compensated) = %f\n", ina_iboost);
  // Gets 5V Boost voltage in V.
  float ina_vboost = M5.Axp.ina3221.getVoltage(INA3221_CH2);
  DEBUG_PRINTF("INA3221: 5V Boost Voltage = %f\n", ina_vboost);

  // Gets bus current in A, compensated with calculated offset voltage.
  float ina_ibus = M5.Axp.ina3221.getCurrentCompensated(INA3221_CH3);
  DEBUG_PRINTF("INA3221: Bus Current (Compensated) = %f\n", ina_ibus);
  // Gets bus voltage in V.
  float ina_vbus = M5.Axp.ina3221.getVoltage(INA3221_CH3);
  DEBUG_PRINTF("INA3221: Bus Voltage = %f\n", ina_vbus);

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

    // Initialize reading from INA3221 ammeter IC
    ina3221_init();
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
