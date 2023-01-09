#ifndef __BATTERY_HPP__
#define __BATTERY_HPP__

/*
** IP5306 Power Module
*/

/* M5 Defaults
KeyOff: Enabled
BoostOutput: Disabled
PowerOnLoad: Enabled
Charger: Enabled
Boost: Enabled
LowBatShutdown: Enabled
ShortPressBoostSwitch: Disabled
FlashlightClicks: Double Press
BoostOffClicks: Long Press
BoostAfterVin: Open
LongPressTime: 2s
ChargeUnderVoltageLoop: 4.55V
ChargeCCLoop: Vin
VinCurrent: 2250mA
VoltagePressure: 28mV
ChargingFullStopVoltage: 4.17V
LightLoadShutdownTime: 32s
EndChargeCurrentDetection: 500mA
ChargeCutoffVoltage: 4.2V
*/

#include <M5Core2.h> // #include <M5Stack.h>

class BatteryMonitorIP5306 {
 private:
  const uint8_t IP5306_I2C_ADDR = 0x75;
  const uint8_t IP5306_REG_SYS_0 = 0x00;
  const uint8_t IP5306_REG_SYS_1 = 0x01;
  const uint8_t IP5306_REG_SYS_2 = 0x02;
  const uint8_t IP5306_REG_CHG_0 = 0x20;
  const uint8_t IP5306_REG_CHG_1 = 0x21;
  const uint8_t IP5306_REG_CHG_2 = 0x22;
  const uint8_t IP5306_REG_CHG_3 = 0x23;
  const uint8_t IP5306_REG_CHG_4 = 0x24;
  const uint8_t IP5306_REG_READ_0 = 0x70;
  const uint8_t IP5306_REG_READ_1 = 0x71;
  const uint8_t IP5306_REG_READ_2 = 0x72;
  const uint8_t IP5306_REG_READ_3 = 0x77;
  const uint8_t IP5306_REG_READ_4 = 0x78;

  uint8_t get_bits(uint8_t reg, uint8_t index, uint8_t bits);
  void set_bits(uint8_t reg, uint8_t index, uint8_t bits, uint8_t value);
  int get_reg(uint8_t reg);
  int set_reg(uint8_t reg, uint8_t value);

 public:
  BatteryMonitorIP5306() {}
  void print_stats();
  void print_settings();
  int8_t get_level();

  uint8_t GetKeyOffEnabled() { return get_bits(IP5306_REG_SYS_0, 0, 1); }
  void SetKeyOffEnabled(uint8_t v) {
    set_bits(IP5306_REG_SYS_0, 0, 1, v);
  }  // 0:dis,*1:en

  uint8_t GetBoostOutputEnabled() { return get_bits(IP5306_REG_SYS_0, 1, 1); }
  void SetBoostOutputEnabled(uint8_t v) {
    set_bits(IP5306_REG_SYS_0, 1, 1, v);  //*0:dis,1:en
  }

  uint8_t GetPowerOnLoadEnabled() { return get_bits(IP5306_REG_SYS_0, 2, 1); }
  void SetPowerOnLoadEnabled(uint8_t v) {
    set_bits(IP5306_REG_SYS_0, 2, 1, v);  // 0:dis,*1:en
  }

  uint8_t GetChargerEnabled() { return get_bits(IP5306_REG_SYS_0, 4, 1); }
  void SetChargerEnabled(uint8_t v) {
    set_bits(IP5306_REG_SYS_0, 4, 1, v);
  }  // 0:dis,*1:en

  uint8_t GetBoostEnabled() { return get_bits(IP5306_REG_SYS_0, 5, 1); }
  void SetBoostEnabled(uint8_t v) {
    set_bits(IP5306_REG_SYS_0, 5, 1, v);  // 0:dis,*1:en
  }

  uint8_t GetLowBatShutdownEnable() { return get_bits(IP5306_REG_SYS_1, 0, 1); }
  void SetLowBatShutdownEnable(uint8_t v) {
    set_bits(IP5306_REG_SYS_1, 0, 1, v);
  }  // 0:dis,*1:en

  uint8_t GetBoostAfterVin() { return get_bits(IP5306_REG_SYS_1, 2, 1); }
  void SetBoostAfterVin(uint8_t v) {
    set_bits(IP5306_REG_SYS_1, 2, 1, v);
  }  // 0:Closed, *1:Open

  uint8_t GetShortPressBoostSwitchEnable() {
    return get_bits(IP5306_REG_SYS_1, 5, 1);
  }
  void SetShortPressBoostSwitchEnable(uint8_t v) {
    set_bits(IP5306_REG_SYS_1, 5, 1, v);
  }  //*0:disabled, 1:enabled

  uint8_t GetFlashlightClicks() { return get_bits(IP5306_REG_SYS_1, 6, 1); }
  void SetFlashlightClicks(uint8_t v) {
    set_bits(IP5306_REG_SYS_1, 6, 1, v);
  }  //*0:short press twice, 1:long press

  uint8_t GetBoostOffClicks() { return get_bits(IP5306_REG_SYS_1, 7, 1); }
  void SetBoostOffClicks(uint8_t v) {
    set_bits(IP5306_REG_SYS_1, 7, 1, v);
  }  //*0:long press, 1:short press twice

  uint8_t GetLightLoadShutdownTime() {
    return get_bits(IP5306_REG_SYS_2, 2, 2);
  }
  void SetLightLoadShutdownTime(uint8_t v) {
    set_bits(IP5306_REG_SYS_2, 2, 2, v);
  }  // 0:8s, *1:32s, 2:16s, 3:64s

  uint8_t GetLongPressTime() { return get_bits(IP5306_REG_SYS_2, 4, 1); }
  void SetLongPressTime(uint8_t v) { set_bits(IP5306_REG_SYS_2, 4, 1, v); }  //*0:2s, 1:3s

  uint8_t GetChargingFullStopVoltage() {
    return get_bits(IP5306_REG_CHG_0, 0, 2);
  }
  void SetChargingFullStopVoltage(uint8_t v) {
    set_bits(IP5306_REG_CHG_0, 0, 2, v);
  }  // 0:4.14V, *1:4.17V, 2:4.185V, 3:4.2V (values are for charge cutoff
     // voltage 4.2V, 0 or 1 is recommended)

  uint8_t GetChargeUnderVoltageLoop() {
    return get_bits(IP5306_REG_CHG_1, 2, 3);
  }  // Automatically adjust the charging current when the voltage of
     // VOUT is greater than the set value
  void SetChargeUnderVoltageLoop(uint8_t v) {
    set_bits(IP5306_REG_CHG_1, 2, 3, v);
  }  // Vout=4.45V + (v * 0.05V) (default 4.55V) //When charging at the maximum
     // current, the charge is less than the set value. Slowly reducing the
     // charging current to maintain this voltage

  uint8_t GetEndChargeCurrentDetection() {
    return get_bits(IP5306_REG_CHG_1, 6, 2);
  }
  void SetEndChargeCurrentDetection(uint8_t v) {
    set_bits(IP5306_REG_CHG_1, 6, 2, v);
  }  // 0:200mA, 1:400mA, *2:500mA, 3:600mA

  uint8_t GetVoltagePressure() { return get_bits(IP5306_REG_CHG_2, 0, 2); }
  void SetVoltagePressure(uint8_t v) {
    set_bits(IP5306_REG_CHG_2, 0, 2, v);
  }  // 0:none, 1:14mV, *2:28mV, 3:42mV (28mV recommended for 4.2V)

  uint8_t GetChargeCutoffVoltage() { return get_bits(IP5306_REG_CHG_2, 2, 2); }
  void SetChargeCutoffVoltage(uint8_t v) {
    set_bits(IP5306_REG_CHG_2, 2, 2, v);
  }  //*0:4.2V, 1:4.3V, 2:4.35V, 3:4.4V

  uint8_t GetChargeCCLoop() { return get_bits(IP5306_REG_CHG_3, 5, 1); }
  void SetChargeCCLoop(uint8_t v) {
    set_bits(IP5306_REG_CHG_3, 5, 1, v);
  }  // 0:BAT, *1:VIN

  uint8_t GetVinCurrent() { return get_bits(IP5306_REG_CHG_4, 0, 5); }
  void SetVinCurrent(uint8_t v) {
    set_bits(IP5306_REG_CHG_4, 0, 5, v);
  }  // ImA=(v*100)+50 (default 2250mA)

  uint8_t GetShortPressDetected() { return get_bits(IP5306_REG_READ_3, 0, 1); }
  void ClearShortPressDetected() {
    return set_bits(IP5306_REG_READ_3, 0, 1, 1);
  }

  uint8_t GetLongPressDetected() { return get_bits(IP5306_REG_READ_3, 1, 1); }
  void ClearLongPressDetected() { return set_bits(IP5306_REG_READ_3, 1, 1, 1); }

  uint8_t GetDoubleClickDetected() { return get_bits(IP5306_REG_READ_3, 2, 1); }
  void ClearDoubleClickDetected() {
    return set_bits(IP5306_REG_READ_3, 2, 1, 1);
  }

  uint8_t GetPowerSource() {
    return get_bits(IP5306_REG_READ_0, 3, 1);
  }  // 0:BAT, 1:VIN
  uint8_t GetBatteryFull() {
    return get_bits(IP5306_REG_READ_1, 3, 1);
  }  // 0:CHG/DIS, 1:FULL
  uint8_t GetOutputLoad() {
    return get_bits(IP5306_REG_READ_2, 2, 1);
  }  // 0:heavy, 1:light
  uint8_t GetLevelLeds() {
    return ((~get_bits(IP5306_REG_READ_4, 4, 4)) &
            0x0F);  // LED[0-4] State (inverted)
  }

  uint8_t LEDS2PCT(uint8_t byte) {
    return ((byte & 0x01 ? 25 : 0) + (byte & 0x02 ? 25 : 0) +
            (byte & 0x04 ? 25 : 0) + (byte & 0x08 ? 25 : 0));
  }
};

#endif  // __BATTERY_HPP__
