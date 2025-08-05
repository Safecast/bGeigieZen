#ifndef BGEIGIEZEN_BATTERY_LED_INDICATOR_H_
#define BGEIGIEZEN_BATTERY_LED_INDICATOR_H_

#include <M5Unified.hpp>
#include <Handler.hpp>
#include "battery_indicator.h"
#include "identifiers.h"

/**
 * LED patterns for different battery states
 */
enum BatteryLedPattern {
  LED_OFF = 0,
  LED_CHARGING_SLOW,    // Green pulsing - slow charging
  LED_CHARGING_FAST,    // Green fast pulsing - fast charging
  LED_FULL_CHARGED,     // Green solid - battery full
  LED_LOW_BATTERY,      // Red blinking - low battery warning
  LED_CRITICAL_BATTERY  // Red fast blinking - critical battery
};

/**
 * Battery LED color structure
 */
struct BatteryLedColor {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  
  BatteryLedColor(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) : red(r), green(g), blue(b) {}
};

/**
 * Battery LED indicator worker that controls LEDs based on battery status
 * Supports both unified M5.Power.setLed() and CoreS3-SE specific GPIO control
 */
class BatteryLedIndicator : public ProcessWorker<bool> {
 public:
  explicit BatteryLedIndicator();
  virtual ~BatteryLedIndicator() = default;

  bool activate(bool retry) override;
  int8_t produce_data(const WorkerMap& workers) override;

 private:
  // LED control methods
  void updateLedPattern();
  void setPowerLed(uint8_t brightness);
  void setPowerLed(const BatteryLedColor& color);
  void setCoreS3SELeds(bool green_on, bool red_on);
  void blinkPattern(bool green_state, bool red_state, uint16_t on_ms, uint16_t off_ms);
  void pulseGreenLed(uint16_t period_ms);
  void pulsePowerLed(uint16_t period_ms, const BatteryLedColor& color);
  void blinkPowerLed(uint16_t period_ms, const BatteryLedColor& color);
  void blinkRedLed(uint16_t period_ms);
  
  // Pattern determination
  BatteryLedPattern determineLedPattern(const BatteryStatus& battery);
  
  // Device detection helpers
  bool isCoreS3SE();
  bool isCore2();
  
  // State tracking
  BatteryLedPattern _current_pattern;
  unsigned long _last_update;
  unsigned long _pattern_start_time;
  bool _led_state;  // For blinking patterns
  bool _is_cores3_se;  // Device type cache
  
  // Configuration
  static const uint16_t UPDATE_INTERVAL_MS = 100;  // 10Hz update rate
  static const uint8_t LOW_BATTERY_THRESHOLD = 20;   // 20% low battery warning
  static const uint8_t CRITICAL_BATTERY_THRESHOLD = 10; // 10% critical battery warning
  
  // CoreS3-SE specific GPIO pins (from schematic analysis)
  static const uint8_t CORES3_SE_GREEN_LED_PIN = 0;   // ESP_BOOT (GPIO0) - Active LOW
  // Red LED is controlled via AXP2101 power management IC
  
  // Color definitions
  static const BatteryLedColor COLOR_OFF;
  static const BatteryLedColor COLOR_CHARGING;     // Orange/yellow
  static const BatteryLedColor COLOR_FULL;         // Green
  static const BatteryLedColor COLOR_LOW;          // Red
  static const BatteryLedColor COLOR_CRITICAL;     // Bright red
};

#endif //BGEIGIEZEN_BATTERY_LED_INDICATOR_H_
