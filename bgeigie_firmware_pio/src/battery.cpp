#include <battery.hpp>

int8_t BatteryMonitorIP5306::get_level() {
  Wire.beginTransmission(IP5306_I2C_ADDR);
  Wire.write(0x78);
  if (Wire.endTransmission(false) == 0 && Wire.requestFrom(IP5306_I2C_ADDR, 1)) {
    switch (Wire.read() & 0xF0) {
      case 0xE0:
        return 25;
      case 0xC0:
        return 50;
      case 0x80:
        return 75;
      case 0x00:
        return 100;
      default:
        return 3;
    }
  }
  return -1;
}

int BatteryMonitorIP5306::get_reg(uint8_t reg) {
  Wire.beginTransmission(IP5306_I2C_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) == 0 && Wire.requestFrom(IP5306_I2C_ADDR, 1)) {
    return Wire.read();
  }
  return -1;
}

int BatteryMonitorIP5306::set_reg(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(IP5306_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value);
  if (Wire.endTransmission(true) == 0) {
    return 0;
  }
  return -1;
}

uint8_t BatteryMonitorIP5306::get_bits(uint8_t reg, uint8_t index,
                                              uint8_t bits) {
  int value = get_reg(reg);
  if (value < 0) {
    Serial.printf("get_bits fail: 0x%02x\n", reg);
    return 0;
  }
  return (value >> index) & ((1 << bits) - 1);
}

void BatteryMonitorIP5306::set_bits(uint8_t reg, uint8_t index,
                                           uint8_t bits, uint8_t value) {
  uint8_t mask = (1 << bits) - 1;
  int v = get_reg(reg);
  if (v < 0) {
    Serial.printf("get_reg fail: 0x%02x\n", reg);
    return;
  }
  v &= ~(mask << index);
  v |= ((value & mask) << index);
  if (set_reg(reg, v)) {
    Serial.printf("set_bits fail: 0x%02x\n", reg);
  }
}

void BatteryMonitorIP5306::print_stats() {
  bool usb = GetPowerSource();
  bool full = GetBatteryFull();
  uint8_t leds = GetLevelLeds();
  Serial.printf(
      "IP5306: Power Source: %s, Battery State: %s, Battery Available: %u%%\n",
      usb ? "USB" : "BATTERY",
      full ? "CHARGED" : (usb ? "CHARGING" : "DISCHARGING"), LEDS2PCT(leds));
}

void BatteryMonitorIP5306::print_settings() {
  delay(1009);
  Serial.println("IP5306 Settings:");
  Serial.printf("  KeyOff: %s\n", GetKeyOffEnabled() ? "Enabled" : "Disabled");
  Serial.printf("  BoostOutput: %s\n",
                GetBoostOutputEnabled() ? "Enabled" : "Disabled");
  Serial.printf("  PowerOnLoad: %s\n",
                GetPowerOnLoadEnabled() ? "Enabled" : "Disabled");
  Serial.printf("  Charger: %s\n",
                GetChargerEnabled() ? "Enabled" : "Disabled");
  Serial.printf("  Boost: %s\n", GetBoostEnabled() ? "Enabled" : "Disabled");
  Serial.printf("  LowBatShutdown: %s\n",
                GetLowBatShutdownEnable() ? "Enabled" : "Disabled");
  Serial.printf("  ShortPressBoostSwitch: %s\n",
                GetShortPressBoostSwitchEnable() ? "Enabled" : "Disabled");
  Serial.printf("  FlashlightClicks: %s\n",
                GetFlashlightClicks() ? "Long Press" : "Double Press");
  Serial.printf("  BoostOffClicks: %s\n",
                GetBoostOffClicks() ? "Double Press" : "Long Press");
  Serial.printf("  BoostAfterVin: %s\n",
                GetBoostAfterVin() ? "Open" : "Not Open");
  Serial.printf("  LongPressTime: %s\n", GetLongPressTime() ? "3s" : "2s");
  Serial.printf("  ChargeUnderVoltageLoop: %.2fV\n",
                4.45 + (GetChargeUnderVoltageLoop() * 0.05));
  Serial.printf("  ChargeCCLoop: %s\n", GetChargeCCLoop() ? "Vin" : "Bat");
  Serial.printf("  VinCurrent: %dmA\n", (GetVinCurrent() * 100) + 50);
  Serial.printf("  VoltagePressure: %dmV\n", GetVoltagePressure() * 14);
  Serial.printf("  ChargingFullStopVoltage: %u\n",
                GetChargingFullStopVoltage());
  Serial.printf("  LightLoadShutdownTime: %u\n", GetLightLoadShutdownTime());
  Serial.printf("  EndChargeCurrentDetection: %u\n",
                GetEndChargeCurrentDetection());
  Serial.printf("  ChargeCutoffVoltage: %u\n", GetChargeCutoffVoltage());
  Serial.println();
}
