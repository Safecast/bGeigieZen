#include "battery_led_indicator.h"
#include <M5Unified.hpp>

// Color definitions
const BatteryLedColor BatteryLedIndicator::COLOR_OFF(0, 0, 0);
const BatteryLedColor BatteryLedIndicator::COLOR_CHARGING(255, 165, 0);  // Orange
const BatteryLedColor BatteryLedIndicator::COLOR_FULL(0, 255, 0);        // Green
const BatteryLedColor BatteryLedIndicator::COLOR_LOW(255, 255, 0);       // Yellow
const BatteryLedColor BatteryLedIndicator::COLOR_CRITICAL(255, 0, 0);    // Red

BatteryLedIndicator::BatteryLedIndicator() 
  : ProcessWorker<bool>(false, UPDATE_INTERVAL_MS),
    _current_pattern(LED_OFF),
    _last_update(0),
    _pattern_start_time(0),
    _led_state(false),
    _is_cores3_se(false) {
}

bool BatteryLedIndicator::activate(bool retry) {
  // Initialize LED to off state
  setPowerLed(0);
  _current_pattern = LED_OFF;
  _pattern_start_time = millis();
  
  // Detect device type and initialize GPIO if needed
  _is_cores3_se = isCoreS3SE();
  
  if (_is_cores3_se) {
    // Initialize GPIO0 for green LED control (active low)
    pinMode(CORES3_SE_GREEN_LED_PIN, OUTPUT);
    digitalWrite(CORES3_SE_GREEN_LED_PIN, HIGH); // Turn OFF (active low)
    M5_LOGI("Battery LED Indicator: CoreS3-SE detected, using GPIO control");
  } else {
    M5_LOGI("Battery LED Indicator: Standard device detected, using M5.Power.setLed()");
  }
  
  // Log device type for debugging
  auto board = M5.getBoard();
  M5_LOGI("Battery LED Indicator activated for board: %d", (int)board);
  
  return true;
}

int8_t BatteryLedIndicator::produce_data(const WorkerMap& workers) {
  // Get battery status from BatteryIndicator worker
  const auto& battery = workers.worker<BatteryIndicator>(k_worker_battery_indicator)->get_data();
  
  // Determine the appropriate LED pattern
  BatteryLedPattern new_pattern = determineLedPattern(battery);
  
  // Update pattern if it changed
  if (new_pattern != _current_pattern) {
    _current_pattern = new_pattern;
    _pattern_start_time = millis();
    _led_state = false;
  }
  
  // Update LED based on current pattern
  updateLedPattern();
  
  data = true;
  return e_worker_data_read;
}

bool BatteryLedIndicator::isCoreS3SE() {
  // For now, we'll detect CoreS3-SE by trying GPIO control
  // This is a safer approach than relying on specific board enum values
  // The user can test and confirm if GPIO0 controls the green LED
  
  // Simple heuristic: if we're on ESP32-S3, assume it might be CoreS3-SE
  #ifdef CONFIG_IDF_TARGET_ESP32S3
    return true;  // Enable GPIO LED control for ESP32-S3 targets
  #else
    return false; // Use standard M5.Power.setLed() for other targets
  #endif
}

bool BatteryLedIndicator::isCore2() {
  // Detect if this is a Core2 (ESP32, not ESP32-S3)
  #ifdef CONFIG_IDF_TARGET_ESP32
    return true;
  #else
    return false;
  #endif
}

BatteryLedPattern BatteryLedIndicator::determineLedPattern(const BatteryStatus& battery) {
  // Critical battery (< 10%)
  if (battery.percentage < CRITICAL_BATTERY_THRESHOLD) {
    return LED_CRITICAL_BATTERY;
  }
  
  // Low battery (< 20%)
  if (battery.percentage < LOW_BATTERY_THRESHOLD) {
    return LED_LOW_BATTERY;
  }
  
  // Charging states
  if (battery.isCharging == m5::Power_Class::is_charging_t::is_charging) {
    // Full battery while charging (>= 95%)
    if (battery.percentage >= 95) {
      return LED_FULL_CHARGED;
    }
    
    // Fast charging (high current)
    if (battery.current_mA < -500) {  // Negative current means charging
      return LED_CHARGING_FAST;
    }
    
    // Normal/slow charging
    return LED_CHARGING_SLOW;
  }
  
  // Not charging, battery full (>= 95%)
  if (battery.percentage >= 95) {
    return LED_FULL_CHARGED;
  }
  
  // Normal battery level, not charging - LED off
  return LED_OFF;
}

void BatteryLedIndicator::updateLedPattern() {
  if (_is_cores3_se) {
    // CoreS3-SE: Use separate green and red LEDs
    switch (_current_pattern) {
      case LED_OFF:
        setCoreS3SELeds(false, false);
        break;
      case LED_CHARGING_SLOW:
        pulseGreenLed(2000); // 2 second pulse
        break;
      case LED_CHARGING_FAST:
        pulseGreenLed(1000); // 1 second pulse
        break;
      case LED_FULL_CHARGED:
        setCoreS3SELeds(true, false); // Solid green
        break;
      case LED_LOW_BATTERY:
        blinkRedLed(1000); // 1 second blink
        break;
      case LED_CRITICAL_BATTERY:
        blinkRedLed(300); // Fast blink
        break;
    }
  } else if (isCore2()) {
    // Core2: Single blue LED - use different patterns for different states
    switch (_current_pattern) {
      case LED_OFF:
        setPowerLed(0);
        break;
      case LED_CHARGING_SLOW:
        pulsePowerLed(2000, BatteryLedColor(0, 0, 255)); // Slow blue pulse for charging
        break;
      case LED_CHARGING_FAST:
        pulsePowerLed(1000, BatteryLedColor(0, 0, 255)); // Fast blue pulse for fast charging
        break;
      case LED_FULL_CHARGED:
        setPowerLed(BatteryLedColor(0, 0, 255)); // Solid blue for full battery
        break;
      case LED_LOW_BATTERY:
        blinkPowerLed(1000, BatteryLedColor(0, 0, 255)); // Slow blue blink for low battery
        break;
      case LED_CRITICAL_BATTERY:
        blinkPowerLed(200, BatteryLedColor(0, 0, 255)); // Very fast blue blink for critical
        break;
    }
  } else {
    // Regular CoreS3: Use unified power LED with full RGB
    switch (_current_pattern) {
      case LED_OFF:
        setPowerLed(0);
        break;
      case LED_CHARGING_SLOW:
        pulsePowerLed(2000, BatteryLedColor(0, 255, 0)); // Green pulse
        break;
      case LED_CHARGING_FAST:
        pulsePowerLed(1000, BatteryLedColor(0, 255, 0)); // Fast green pulse
        break;
      case LED_FULL_CHARGED:
        setPowerLed(BatteryLedColor(0, 255, 0)); // Solid green
        break;
      case LED_LOW_BATTERY:
        blinkPowerLed(1000, BatteryLedColor(255, 0, 0)); // Red blink
        break;
      case LED_CRITICAL_BATTERY:
        blinkPowerLed(300, BatteryLedColor(255, 0, 0)); // Fast red blink
        break;
    }
  }
}

void BatteryLedIndicator::setPowerLed(uint8_t brightness) {
  // Use M5Unified power LED control for standard devices
  M5.Power.setLed(brightness);
}

void BatteryLedIndicator::setPowerLed(const BatteryLedColor& color) {
  // For Core2, we only have blue LED, so use brightness based on any color component
  if (isCore2()) {
    // Use the blue component or average of RGB for brightness
    uint8_t brightness = color.blue > 0 ? color.blue : (color.red + color.green + color.blue) / 3;
    M5.Power.setLed(brightness);
  } else {
    // For other devices, this would set RGB if supported
    // For now, use brightness average
    uint8_t brightness = (color.red + color.green + color.blue) / 3;
    M5.Power.setLed(brightness);
  }
}

void BatteryLedIndicator::pulsePowerLed(uint16_t period_ms, const BatteryLedColor& color) {
  unsigned long current_time = millis();
  unsigned long elapsed = current_time - _pattern_start_time;
  unsigned long cycle_time = elapsed % period_ms;
  
  // Create sine wave for smooth pulsing
  float phase = (float)cycle_time / period_ms * 2.0 * PI;
  float intensity = (sin(phase) + 1.0) / 2.0; // 0 to 1
  
  BatteryLedColor pulsed_color;
  pulsed_color.red = (uint8_t)(color.red * intensity);
  pulsed_color.green = (uint8_t)(color.green * intensity);
  pulsed_color.blue = (uint8_t)(color.blue * intensity);
  
  setPowerLed(pulsed_color);
}

void BatteryLedIndicator::blinkPowerLed(uint16_t period_ms, const BatteryLedColor& color) {
  unsigned long current_time = millis();
  unsigned long elapsed = current_time - _pattern_start_time;
  unsigned long cycle_time = elapsed % (period_ms * 2); // On + Off period
  
  if (cycle_time < period_ms) {
    setPowerLed(color); // On
  } else {
    setPowerLed(COLOR_OFF); // Off
  }
}

void BatteryLedIndicator::blinkRedLed(uint16_t period_ms) {
  // For CoreS3-SE, blink the red LED via power management
  unsigned long current_time = millis();
  unsigned long elapsed = current_time - _pattern_start_time;
  unsigned long cycle_time = elapsed % (period_ms * 2);
  
  if (cycle_time < period_ms) {
    // Red LED on - this would need AXP2101 control for CoreS3-SE
    // For now, fall back to power LED
    setPowerLed(100);
  } else {
    setPowerLed(0); // Off
  }
}

void BatteryLedIndicator::setCoreS3SELeds(bool green_on, bool red_on) {
  // Control CoreS3-SE specific LEDs
  
  // Green LED control (GPIO0, active low)
  digitalWrite(CORES3_SE_GREEN_LED_PIN, green_on ? LOW : HIGH);
  
  // Red LED control (via AXP2101 power management)
  // For now, we'll use the M5.Power.setLed for red indication
  // This may need refinement based on actual hardware testing
  if (red_on) {
    M5.Power.setLed(150);  // Medium brightness for red indication
  } else if (!green_on) {
    M5.Power.setLed(0);    // Turn off only if both LEDs should be off
  }
}

void BatteryLedIndicator::blinkPattern(bool green_state, bool red_state, uint16_t on_ms, uint16_t off_ms) {
  unsigned long current_time = millis();
  unsigned long elapsed = current_time - _pattern_start_time;
  unsigned long cycle_time = on_ms + off_ms;
  unsigned long position_in_cycle = elapsed % cycle_time;
  
  bool should_be_on = position_in_cycle < on_ms;
  
  if (should_be_on != _led_state) {
    _led_state = should_be_on;
    
    if (_is_cores3_se) {
      if (should_be_on) {
        setCoreS3SELeds(green_state, red_state);
      } else {
        setCoreS3SELeds(false, false);
      }
    } else {
      if (should_be_on) {
        setPowerLed(150);  // Medium-high brightness when on
      } else {
        setPowerLed(0);    // Off
      }
    }
  }
}

void BatteryLedIndicator::pulseGreenLed(uint16_t period_ms) {
  unsigned long current_time = millis();
  unsigned long elapsed = current_time - _pattern_start_time;
  unsigned long position_in_cycle = elapsed % period_ms;
  
  // Create sine wave for smooth pulsing
  float phase = (2.0f * PI * position_in_cycle) / period_ms;
  float intensity = (sin(phase) + 1.0f) / 2.0f;  // 0.0 to 1.0
  
  if (_is_cores3_se) {
    // For CoreS3-SE, we'll use a simple on/off pattern since GPIO doesn't support PWM easily
    bool green_on = intensity > 0.5f;
    setCoreS3SELeds(green_on, false);
  } else {
    // Map to LED brightness range (20-200 for visibility)
    uint8_t brightness = 20 + (uint8_t)(intensity * 180);
    setPowerLed(brightness);
  }
}
