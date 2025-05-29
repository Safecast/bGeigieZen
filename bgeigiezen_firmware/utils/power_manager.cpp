#include "power_manager.h"
#include <Wire.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <soc/rtc.h>
#include <esp32-hal-cpu.h>

// Initialize static members
bool PowerManager::_low_power_mode = false;
uint32_t PowerManager::_original_cpu_freq = 240; // Default to 240MHz
uint32_t PowerManager::_original_i2c_freq = 100000; // Default to 100kHz

void PowerManager::begin() {
    // Store original CPU frequency
    rtc_cpu_freq_config_t config;
    rtc_clk_cpu_freq_get_config(&config);
    _original_cpu_freq = config.freq_mhz;
    
    // Store original I2C frequency (default is usually 100kHz)
    _original_i2c_freq = 100000;  // Will be updated in setI2cClock if different
}

void PowerManager::enterLowPowerMode() {
    if (_low_power_mode) return;  // Already in low power mode
    
    M5_LOGI("Entering low power mode");
    
    // 1. Reduce CPU frequency first to save power
    setCpuFrequency(80);  // 80MHz is a good balance between power and performance
    
    // 2. Reduce I2C clock speed
    setI2cClock(50000);  // 50kHz is sufficient for most sensors
    
    // 3. Disable WiFi and Bluetooth
    disableWireless();
    
    // 4. Reduce logging to minimum
    esp_log_level_set("*", ESP_LOG_ERROR);
    
    _low_power_mode = true;
    M5_LOGI("Low power mode activated");
}

void PowerManager::exitLowPowerMode() {
    if (!_low_power_mode) return;  // Not in low power mode
    
    M5_LOGI("Exiting low power mode");
    
    // 1. Restore CPU frequency
    if (_original_cpu_freq > 0) {
        setCpuFrequency(_original_cpu_freq);
    }
    
    // 2. Restore I2C clock speed
    if (_original_i2c_freq > 0) {
        setI2cClock(_original_i2c_freq);
    }
    
    // Restore normal logging
    esp_log_level_set("*", ESP_LOG_INFO);
    
    // Note: WiFi/BT will be re-enabled when needed by the specific mode
    
    _low_power_mode = false;
    M5_LOGI("Normal power mode restored");
}

bool PowerManager::setCpuFrequency(uint32_t freq_mhz) {
    // Store the original frequency if not already stored
    if (_original_cpu_freq == 0) {
        _original_cpu_freq = getCpuFrequency();
    }

    // Check if frequency is already set
    if (getCpuFrequency() == freq_mhz) {
        return true;
    }
    
    M5_LOGI("Changing CPU frequency from %lu MHz to %lu MHz", 
            (unsigned long)getCpuFrequency(), (unsigned long)freq_mhz);
    
    // Set CPU frequency using Arduino-ESP32 API
    bool success = setCpuFrequencyMhz(freq_mhz);
    
    if (!success) {
        M5_LOGE("Failed to set CPU frequency to %lu MHz", (unsigned long)freq_mhz);
        return false;
    }
    
    // Verify
    if (getCpuFrequency() != freq_mhz) {
        M5_LOGE("Failed to verify CPU frequency set to %lu MHz", (unsigned long)freq_mhz);
        return false;
    }
    
    return true;
}

uint32_t PowerManager::getCpuFrequency() {
    return getCpuFrequencyMhz();
}

void PowerManager::disableWireless() {
    M5_LOGI("Disabling wireless modules");
    
    // Disable WiFi
    WiFi.mode(WIFI_OFF);
    esp_wifi_stop();
    esp_wifi_deinit();
  
    // Disable Bluetooth if it was enabled
    if (btStarted()) {
        btStop();
    }
}

void PowerManager::enableWireless() {
    M5_LOGI("Wireless modules will be re-enabled when needed by specific modes");
    // Note: WiFi and BT are enabled on-demand by the specific features that need them
}

void PowerManager::setI2cClock(uint32_t freq_hz) {
    if (freq_hz < 10000 || freq_hz > 1000000) {
        M5_LOGW("I2C frequency %u Hz is outside recommended range (10kHz - 1MHz)", freq_hz);
    }
    
    if (!_low_power_mode) {
        // Only update original frequency if not in low power mode
        _original_i2c_freq = Wire.getClock();
    }
    
    M5_LOGI("Setting I2C clock to %u Hz (was %u Hz)", freq_hz, Wire.getClock());
    Wire.setClock(freq_hz);
}

// Brownout detector control removed for ESP32-S3 compatibility
// The brownout detector is a safety feature that prevents damage
// from low voltage conditions. It's better to keep it enabled.
