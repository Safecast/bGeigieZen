#pragma once

#include <M5Unified.hpp>

// Minimal helper utilities for configuring the AXP2101 fuel-gauge on M5Stack Core2 v1.1.
// These operate directly on the same I2C instance used by M5.Power.
// NOTE: Register addresses are taken from the AXP2101 datasheet v1.0.
// If a future library adds first-class support, replace this file.

namespace axp2101 {

constexpr uint8_t AXP2101_I2C_ADDR          = 0x34; // default slave address

// Register addresses (high byte first when multi-byte)
constexpr uint8_t REG_COULOMB_CTL           = 0xB8; // enable / reset coulomb counter
constexpr uint8_t REG_BAT_CAP_H             = 0xB0; // advertised battery capacity high byte
constexpr uint8_t REG_BAT_CAP_L             = 0xB1; // advertised battery capacity low byte

// Bits for REG_COULOMB_CTL
constexpr uint8_t BIT_CC_ENABLE             = 0x80; // enable coulomb counter
constexpr uint8_t BIT_CC_CLR                = 0x40; // clear accumulators

inline bool write_register(uint8_t reg, uint8_t value) {
  constexpr uint32_t I2C_FREQ_HZ = 100000; // keep default bus speed
  return M5.In_I2C.writeRegister8(AXP2101_I2C_ADDR, reg, value, I2C_FREQ_HZ);
}

inline bool read_register(uint8_t reg, uint8_t &value) {
  constexpr uint32_t I2C_FREQ_HZ = 100000;
  value = M5.In_I2C.readRegister8(AXP2101_I2C_ADDR, reg, I2C_FREQ_HZ);
  return true; // M5 API does not expose error state
}

// Remaining battery capacity registers (AXP2101 datasheet suggests 0xB2/0xB3)
constexpr uint8_t REG_REMAIN_CAP_H = 0xB2;
constexpr uint8_t REG_REMAIN_CAP_L = 0xB3;

inline float get_remaining_capacity_percent(uint16_t programmed_capacity_mAh) {
  uint8_t high, low;
  if (!read_register(REG_REMAIN_CAP_H, high)) return -1.0f;
  if (!read_register(REG_REMAIN_CAP_L, low))  return -1.0f;
  uint16_t raw = (static_cast<uint16_t>(high) << 8) | low;
  if (programmed_capacity_mAh == 0) return -1.0f;
  return (static_cast<float>(raw) * 100.0f) / programmed_capacity_mAh;
}

inline bool configure_coulomb_counter(uint16_t capacity_mAh) {
  // Program advertised capacity.
  if (!write_register(REG_BAT_CAP_H, static_cast<uint8_t>(capacity_mAh >> 8))) return false;
  if (!write_register(REG_BAT_CAP_L, static_cast<uint8_t>(capacity_mAh & 0xFF))) return false;

  // Clear existing data then enable counter.
  if (!write_register(REG_COULOMB_CTL, BIT_CC_CLR)) return false; // pulse clear
  if (!write_register(REG_COULOMB_CTL, BIT_CC_ENABLE)) return false;
  return true;
}

} // namespace axp2101
